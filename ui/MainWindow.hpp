
#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

namespace tbui {

class MainWindow : public wxFrame {
public:
    MainWindow();

private:
    static constexpr int ID_New = 1;
    static constexpr int ID_Open = 2;
    static constexpr int ID_Save = 3;
    static constexpr int ID_SaveAs = 4;


    void OnNew(wxCommandEvent &evt);
    void OnOpen(wxCommandEvent &evt);
    void OnSave(wxCommandEvent &evt);
    void OnSaveAs(wxCommandEvent &evt);
    void OnExit(wxCommandEvent &evt);
    void OnAbout(wxCommandEvent &evt);
};


}
