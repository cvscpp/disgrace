#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "core/engine.h"
#include "gui/main_window.h"

int main(int argc, char **argv)
{
    try
    {
        dg::Engine engine;

        if (!engine.initialize())
            return 1;

        dg::MainWindow window(1024, 768, "Disgrace", engine);
        window.set_engine(&engine);
        window.show(argc, argv);

        int ret = Fl::run();

        engine.shutdown();
        return ret;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }
}

