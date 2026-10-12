#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

namespace disgrace_ns {

// Worker thread pool for parallel track rendering.
//
// Key design: m_done counts *completed items*, not workers that woke up.
// parallel_for() only blocks until all count items are done, then returns
// immediately — it does NOT wait for idle workers to acknowledge the epoch.
//
// This means: if the calling RT thread drains all items itself before any
// worker thread even wakes, parallel_for returns with zero wait time and
// zero mutex operations in the fast path.

class AudioThreadPool {
public:
    explicit AudioThreadPool(size_t num_workers = 0) {
        if (num_workers > 0)
            start_workers(num_workers);
    }

    ~AudioThreadPool() { stop_all(); }

    AudioThreadPool(const AudioThreadPool&) = delete;
    AudioThreadPool& operator=(const AudioThreadPool&) = delete;

    size_t size() const { return m_workers.size(); }

    void resize(size_t num_workers) {
        stop_all();
        if (num_workers > 0)
            start_workers(num_workers);
    }

    // Execute fn(i) for i in [0, count) in parallel.
    // Template (no std::function type-erasure): avoids a heap allocation
    // per audio block when the caller passes a lambda. The callable must
    // stay alive until parallel_for returns (it always does — we join).
    // Small fan-outs run serially: mutex + wakeups + atomics cost more
    // than the work for a handful of tracks. Never blocks the RT thread
    // longer than a few ms (dropout instead of a 20 ms stall).
    template <typename Fn>
    void parallel_for(size_t count, Fn&& fn) {
        if (count == 0) return;
        if (m_workers.empty() || count <= 4) {
            for (size_t i = 0; i < count; ++i) fn(i);
            return;
        }

        using FnDecay = typename std::decay<Fn>::type;
        struct Helper {
            static void call(void* p, size_t i) { (*static_cast<FnDecay*>(p))(i); }
        };

        // Publish new work under the mutex so workers see consistent state.
        {
            std::lock_guard<std::mutex> lk(m_mu);
            m_tramp = &Helper::call;
            m_ctx   = static_cast<void*>(&fn);
            m_total = count;
            m_next.store(0, std::memory_order_relaxed);
            m_done.store(0, std::memory_order_relaxed);
            ++m_epoch;
        }
        m_work_cv.notify_all();

        // RT thread drains as many items as it can.
        drain_direct(count, fn);

        // Fast path: all items done by the RT thread, no wait needed.
        if (m_done.load(std::memory_order_acquire) >= count)
            return;

        // Slow path: some items were taken by workers; wait briefly.
        // Short timeout so a stuck worker causes a dropout, never a freeze.
        std::unique_lock<std::mutex> lk(m_mu);
        m_done_cv.wait_for(lk, std::chrono::milliseconds(5), [&] {
            return m_done.load(std::memory_order_relaxed) >= count;
        });
    }

private:
    // RT-side drain calling the local callable directly (no indirection).
    template <typename Fn>
    void drain_direct(size_t total, Fn&& fn) {
        for (;;) {
            size_t idx = m_next.fetch_add(1, std::memory_order_relaxed);
            if (idx >= total) break;
            fn(idx);
            // Signal if this was the last item.
            if (m_done.fetch_add(1, std::memory_order_acq_rel) + 1 >= total)
                m_done_cv.notify_one();
        }
    }

    // Worker-side drain via the type-erased trampoline.
    void drain_tramp(size_t total) {
        Trampoline tramp = m_tramp;
        void* ctx = m_ctx;
        for (;;) {
            size_t idx = m_next.fetch_add(1, std::memory_order_relaxed);
            if (idx >= total) break;
            tramp(ctx, idx);
            // Signal if this was the last item.
            if (m_done.fetch_add(1, std::memory_order_acq_rel) + 1 >= total)
                m_done_cv.notify_one();
        }
    }

    void start_workers(size_t n) {
        m_stop = false;
        m_workers.reserve(n);
        for (size_t i = 0; i < n; ++i)
            m_workers.emplace_back([this] { worker_loop(); });
    }

    void stop_all() {
        {
            std::lock_guard<std::mutex> lk(m_mu);
            m_stop = true;
            ++m_epoch;
        }
        m_work_cv.notify_all();
        for (auto& t : m_workers)
            if (t.joinable()) t.join();
        m_workers.clear();
        m_stop = false;
    }

    void worker_loop() {
        int seen_epoch;
        {
            std::lock_guard<std::mutex> lk(m_mu);
            seen_epoch = m_epoch;
        }

        while (true) {
            // Wait for new work (epoch change) or shutdown.
            {
                std::unique_lock<std::mutex> lk(m_mu);
                m_work_cv.wait(lk, [&] {
                    return m_stop || m_epoch != seen_epoch;
                });
                if (m_stop) return;
                seen_epoch = m_epoch;
            }
            // m_total/tramp/ctx are stable for this epoch (written before
            // ++m_epoch under the same lock, visible after acquiring above).
            const size_t total = m_total;

            // Drain: process whatever items remain, one m_done++ per item.
            drain_tramp(total);
            // No per-worker done signal needed — only per-item counts.
        }
    }

    std::vector<std::thread>            m_workers;
    std::mutex                          m_mu;
    std::condition_variable             m_work_cv;
    std::condition_variable             m_done_cv;
    bool                                m_stop{false};
    int                                 m_epoch{0};
    using Trampoline = void(*)(void*, size_t);
    Trampoline                          m_tramp{nullptr};
    void*                               m_ctx{nullptr};
    size_t                              m_total{0};
    // Padded: RT thread and workers hammer these concurrently.
    alignas(64) std::atomic<size_t>     m_next{0};
    alignas(64) std::atomic<size_t>     m_done{0};
};

} // namespace disgrace_ns
