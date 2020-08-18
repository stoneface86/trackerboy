
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "MainWindow.hpp"
using tbui::MainWindow;


class App : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    MainWindow *win = new MainWindow();
    win->Show(true);

    return true;
}

