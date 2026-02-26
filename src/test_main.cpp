#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <cstdio> // For printf

int main(int argc, char **argv) {
    printf("Starting FLTK test application\n"); // Corrected line
    Fl_Double_Window window(300, 200, "Test Window");
    window.begin();
    Fl_Box box(20, 20, 260, 160, "Hello, FLTK!");
    window.end();
    window.show(argc, argv);
    printf("FLTK test application running\n"); // Corrected line
    return Fl::run();
}