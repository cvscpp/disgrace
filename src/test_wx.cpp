#include <wx/wxprec.h>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/strconv.h>
#include <iostream>
#include <locale.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit() override {
        std::cout << "Test: OnInit" << std::endl;
        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Test");
        std::cout << "Test: Frame created" << std::endl;
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP_NO_MAIN(MyApp);

int main(int argc, char** argv) {
    setenv("LANG", "C.UTF-8", 1);
    setenv("LC_ALL", "C.UTF-8", 1);
    setlocale(LC_ALL, "C.UTF-8");
    
    // Force UTF-8 converter before any wx objects are created
    wxConvCurrent = &wxConvUTF8;
    
    return wxEntry(argc, argv);
}
