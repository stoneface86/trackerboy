
# Ui project

This subdirectory contains the ui project. You can build this project via the
root CMakeLists.txt, with TRACKERBOY_ENABLE_UI option set to ON.

## Organization

* `ui/`
  * `designer/` - Qt Designer files (.ui) go here
  * `forms/` - Form/dialog classes
  * `model/` - data model classes using Qt Model/View framework
  * `resources/` - images, Qt resource files (.qrc), etc
  * `widgets/` - QWidget subclasses and custom widgets/controls

## How to...

### Add a Qt designer widget

We use the pointer member variable approach or PIMPL idiom. See
https://doc.qt.io/qt-5/designer-using-a-ui-file.html#using-a-pointer-member-variable for more details.

1. Design your widget with designer, save the ui file in the designer folder
2. Add the ui file to the UI_DESIGNER list in CMakeLists.txt
3. Add this header to forms or widgets subdirectory, replacing 
`YourWidget` with the widget's class name

```cpp
#pragma once

#include <QWidget>

namespace Ui {
class YourWidget;
}

class YourWidget : public QWidget {

    Q_OBJECT

public:
    explicit YourWidget(QWidget *parent = nullptr);
    virtual ~YourWidget();

private:
    Ui::YourWidget *mUi;
};
```
4. Add this source file to the same directory as step 3, 
replacing `YourWidget` with the widget's class name.

```cpp

#include "widgets/YourWidget.hpp"

#pragma warning(push, 0)
#include "ui_YourWidget.h"
#pragma warning(pop)

YourWidget::YourWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::YourWidget())
{
    mUi->setupUi(this);
}

YourWidget::~YourWidget() {
    delete mUi;
}
```

5. Finally, add the source file path to the CMakeLists.txt, via the
UI_SRC variable (please keep in alphabetical order). Rebuild the
project (if you don't you may get linker errors due to AUTOMOC not
working).

