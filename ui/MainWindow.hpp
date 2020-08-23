
#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/docview.h>

namespace tbui {

class MainWindow : public wxDocParentFrame {
public:
    MainWindow(wxDocManager *manager);

private:

    void OnExit(wxCommandEvent &evt);
    void OnAbout(wxCommandEvent &evt);


    wxDECLARE_EVENT_TABLE();

};


}
