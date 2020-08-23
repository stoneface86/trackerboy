
#include <sstream>

#include "TrackerboyApp.hpp"
#include "MainWindow.hpp"
#include "ModuleView.hpp"

using tbui::ModuleView;
wxIMPLEMENT_DYNAMIC_CLASS(ModuleView, wxView);

namespace tbui {


void ModuleView::OnDraw(wxDC *dc) {

}

void ModuleView::OnChangeFilename() {
    auto &app = wxGetApp();
    app.setDocumentTitle(m_viewDocument->GetUserReadableName());
}

}
