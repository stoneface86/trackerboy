
#include <sstream>

#include <wx/config.h>

#include "MainWindow.hpp"

using tbui::MainWindow;
wxBEGIN_EVENT_TABLE(MainWindow, wxDocParentFrame)
    EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
    EVT_MENU(wxID_EXIT, MainWindow::OnExit)
wxEND_EVENT_TABLE()


namespace tbui {

MainWindow::MainWindow(wxDocManager *manager) :
    wxDocParentFrame(manager, nullptr, wxID_ANY, "")
{

    // MENU ===============================================================
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW);
    menuFile->SetHelpString(wxID_NEW, "Start a new module");
    menuFile->Append(wxID_OPEN);
    menuFile->SetHelpString(wxID_OPEN, "Open an existing module");
    menuFile->Append(wxID_SAVE);
    menuFile->SetHelpString(wxID_SAVE, "Save the current module");
    menuFile->Append(wxID_SAVEAS);
    menuFile->SetHelpString(wxID_SAVEAS, "Save the current module as a new file");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    manager->FileHistoryLoad(*wxConfig::Get());
    manager->FileHistoryUseMenu(menuFile);
    manager->FileHistoryAddFilesToMenu();

    wxMenu *menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO);
    menuEdit->Append(wxID_REDO);

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    // LAYOUT =========================================================

    //wxBoxSizer *hboxSizer = new wxBoxSizer(wxHORIZONTAL);


    // STATUS BAR ======================================================
    CreateStatusBar();
    SetStatusText("Ready");
    
}

void MainWindow::OnAbout(wxCommandEvent &evt) {
    wxMessageBox("Trackerboy v0.0.6\n\nCopyright 2020 - stoneface86", "About", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnExit(wxCommandEvent &evt) {
    Close(true);
}

}
