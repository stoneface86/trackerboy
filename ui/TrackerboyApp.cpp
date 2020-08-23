
#include <sstream>

#include "TrackerboyApp.hpp"
#include <wx/docview.h>
#include <wx/config.h>

#include "MainWindow.hpp"
using tbui::MainWindow;

#include "docview/ModuleDocument.hpp"
using tbui::ModuleDocument;

#include "docview/ModuleView.hpp"
using tbui::ModuleView;


wxIMPLEMENT_APP(TrackerboyApp);

bool TrackerboyApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }

    mLastModifiedState = false;

    SetVendorName("trackerboy");
    SetAppName("trackerboy");
    SetAppDisplayName("Trackerboy");

    wxDocManager *docMan = new wxDocManager;

    new wxDocTemplate(
        docMan,
        "Trackerboy Module",
        "*.tbm",
        "",
        "tbm",
        "Module doc",
        "Module view",
        CLASSINFO(ModuleDocument),
        CLASSINFO(ModuleView)
    );

    // SDI so only one document may be open
    docMan->SetMaxDocsOpen(1);

    mWindow = new MainWindow(docMan);

    // always start with a new document
    docMan->CreateNewDocument();

    mWindow->Show(true);

    return true;
}

int TrackerboyApp::OnExit() {
    auto docMan = wxDocManager::GetDocumentManager();
    docMan->FileHistorySave(*wxConfig::Get());
    delete docMan;

    return wxApp::OnExit();
}

void TrackerboyApp::setDocumentTitle(wxString str) {
    mDocumentTitle = str;
    updateTitle();
    
}

void TrackerboyApp::setDocumentModified(bool modified) {
    if (modified != mLastModifiedState) {
        mLastModifiedState = modified;
        updateTitle();
    }
}

void TrackerboyApp::updateTitle() {
    std::ostringstream out;

    out << mDocumentTitle;
    if (mLastModifiedState) {
        out << "*";
    }
    out << " - Trackerboy";

    mWindow->SetTitle(out.str());
}

