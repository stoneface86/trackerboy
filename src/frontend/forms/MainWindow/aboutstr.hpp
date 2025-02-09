#pragma once
class QString;

//
// Gets the about string to be passed to the About dialog. Note that the format
// of this is string is rich text (HTML). The content of the string contains
// version information, as well as the git commit.
//
extern QString getAboutString();
