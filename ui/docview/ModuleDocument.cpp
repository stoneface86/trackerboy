
#include "ModuleDocument.hpp"

using tbui::ModuleDocument;
wxIMPLEMENT_DYNAMIC_CLASS(ModuleDocument, wxDocument);

namespace tbui {

bool ModuleDocument::DeleteContents() {
    mModule.clear();
    return true;
}

Module& ModuleDocument::getModule() {
    return mModule;
}

std::istream& ModuleDocument::LoadObject(std::istream &stream) {
    mLastError = mFile.loadHeader(stream);
    if (mLastError == trackerboy::FormatError::none) {
        mLastError = mFile.loadModule(stream, mModule);
    }
    return stream;
}

std::ostream& ModuleDocument::SaveObject(std::ostream &stream) {
    mLastError = mFile.saveHeader(stream);
    if (mLastError == trackerboy::FormatError::none) {
        mLastError = mFile.saveModule(stream, mModule);
    }
    return stream;
}

}
