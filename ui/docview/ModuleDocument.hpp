
#include <ostream>
#include <wx/docview.h>

#include "trackerboy/data/Module.hpp"
using trackerboy::Module;

#include "trackerboy/File.hpp"
using trackerboy::File;


namespace tbui {

class ModuleDocument : public wxDocument {

public:

    bool DeleteContents() override;

    std::istream& LoadObject(std::istream &stream) override;

    Module& getModule();

    std::ostream& SaveObject(std::ostream &stream) override;

private:
    Module mModule;
    File mFile;

    trackerboy::FormatError mLastError;

    wxDECLARE_DYNAMIC_CLASS(ModuleDocument);

};


}
