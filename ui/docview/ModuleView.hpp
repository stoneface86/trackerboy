
#pragma once

#include <wx/docview.h>

namespace tbui {

class ModuleView : public wxView {

public:

    void OnDraw(wxDC *dc) override;

    void OnChangeFilename() override;

private:

    wxDECLARE_DYNAMIC_CLASS(ModuleView);
};


}
