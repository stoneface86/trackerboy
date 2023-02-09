
# qt bindings

This folder contains an incomplete set of bindings for QtCore, QtGui and
QtWidgets. Since there are not much options for Nim Qt bindings, we will only
bind what we need.

Modules are organized similarly to how Qt's includes are organized. For example,
`QObject` is located in header `QtCore/qobject.h`, its Nim bindings are located
in module `qt/core/qobject.nim`.
