#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "core/engine.h"
#include "gui/main_window.h"

int main(int argc, char **argv)
{
    try
    {
        disgrace_ns::Engine engine;

        if (!engine.initialize())
            return 1;

        disgrace_ns::MainWindow window(1024, 768, "Disgrace", engine);
        window.show(argc, argv);

        Fl::add_timeout(0.03,
                        [](void* userdata)
                        {
                            auto* self = static_cast<disgrace_ns::MainWindow*>(userdata);
                            self->redraw();
                            Fl::repeat_timeout(0.03,
                                               disgrace_ns::MainWindow::timer_cb,
                                               userdata);
                        },
                        &window); // Pass the window object itself as userdata

        int ret = Fl::run();

        engine.shutdown();
        return ret;
    }
    catch (const ::std::exception &e)
    {
        fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }
}
