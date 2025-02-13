
#include "forms/MainWindow/aboutstr.hpp"
#include "version.hpp"

#include "tbb.h"

#include <QString>
#include <QtGlobal>

#include "miniaudio.h"
#include "RtMidi.h"

QString getAboutString() {

    static constexpr auto cTemplate = R"html(
<html><head/><body>
<h1>Trackerboy v%1</h1>
<h3>Game Boy / Game Boy Color music tracker</h3>
<h3>Copyright (C) 2019-2025 stoneface86</h3>
<p>
<a href="https://www.trackerboy.org">www.trackerboy.org</a>
<br>
Repo - <a href="https://github.com/stoneface86/trackerboy">https://github.com/stoneface86/trackerboy</a>
<br>
Commit: %2
</p>
<p>This software is licensed under the MIT License.</p>
<hr>
<h3>Libraries</h3>
<ul>
<li>libtrackerboy %3</li>
<li>Qt %4</li>
<li>Miniaudio %5 (C) David Reid</li>
<li>RtMidi %6 (C) Gary P. Scavone</li>
</ul></body></html>
)html";
    return QString(cTemplate).arg(
        cVersion,
        cCommit,
        bVersion(),
        qVersion(),
        ma_version_string(),
        QString::fromStdString(RtMidi::getVersion())
    );

}

