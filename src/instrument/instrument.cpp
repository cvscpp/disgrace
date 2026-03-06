#include "../core/engine.h" // ADDED
#include <cmath> // Added for powf
#include "instrument.h"

namespace disgrace_ns
{

    disgrace_ns::Voice* disgrace_ns::Instrument::allocate_voice(size_t column_index)
    {
        // find inactive voice
        for (auto& v : m_voices)
        {
            if (!v) {
                v = create_voice();
                if (v) {
                    v->set_column(column_index);
                    return v.get();
                }
                continue;
            }

            if (!v->active()) {
                v->set_column(column_index);
                return v.get();
            }
        }

        // voice stealing: steal oldest (voice 0)
        if (m_voices[0]) {
            m_voices[0]->set_column(column_index);
            return m_voices[0].get();
        }
        return nullptr;
    }

} // namespace disgrace_ns
