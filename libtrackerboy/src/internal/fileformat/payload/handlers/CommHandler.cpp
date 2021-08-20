#include "internal/fileformat/payload/handlers/CommHandler.hpp"

namespace trackerboy {

CommHandler::CommHandler() :
    PayloadHandler(1)
{
}

FormatError CommHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)index;
    auto commentsLength = block.size();
    if (commentsLength) {
        std::string comments;
        comments.resize(commentsLength);
        block.read(commentsLength, comments.data());
        mod.setComments(std::move(comments));
    }

    return FormatError::none;
}

void CommHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)index;

    auto const& comments = mod.comments();
    block.write(comments.size(), comments.data());
}

}
