
#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "MainWindow.hpp"

// naming conventions
// wxWidgets uses PascalCase for functions/methods
// we will keep using camelCase (so we can tell which methods are ours)


// The App class is in the global namespace, everything else exists in tbui


class TrackerboyApp : public wxApp {
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

    // name of the document to show in the title bar
    void setDocumentTitle(wxString str);

    void setDocumentModified(bool modified);

private:
    tbui::MainWindow *mWindow;
    wxString mDocumentTitle;
    bool mLastModifiedState;

    void updateTitle();
};


wxDECLARE_APP(TrackerboyApp);
