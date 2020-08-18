
#include "MainWindow.hpp"

namespace tbui {

MainWindow::MainWindow() :
    wxFrame(NULL, wxID_ANY, "TrackerBoy")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_New, "&New\tCtrl-N", "Start a new module");
    menuFile->Append(ID_Open, "&Open\tCtrl-O", "Open a module file");
    menuFile->Append(ID_Save, "&Save\tCtrl-S", "Save the module");
    menuFile->Append(ID_SaveAs, "&Save As...", "Save module to a new file");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Ready");

    Bind(wxEVT_MENU, &MainWindow::OnNew, this, ID_New);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);


}

void MainWindow::OnNew(wxCommandEvent &evt) {
    wxLogMessage("New file");
}

void MainWindow::OnOpen(wxCommandEvent &evt) {

}

void MainWindow::OnSave(wxCommandEvent &evt) {

}

void MainWindow::OnSaveAs(wxCommandEvent &evt) {

}

void MainWindow::OnAbout(wxCommandEvent &evt) {
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello world", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnExit(wxCommandEvent &evt) {
    Close(true);
}

}
