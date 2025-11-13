
#include "widgets/DataWidget.hxx"
#include "core/icons.hxx"
#include "utils/actions.hxx"
#include "utils/connectutils.hxx"

#include <QVBoxLayout>

DataWidget::DataWidget(NameListModel *model, QString const &title,
                       QWidget *parent)
    : QGroupBox(title, parent)
    , _model(new TableModel(model, this))
    , _toolbar(nullptr)
    , _view(nullptr)
    , _actions()
    , _currentId(-1) {

    _toolbar = icons::smallToolBar();
    _view = new QListView;
    _view->setWrapping(true);
    _view->setResizeMode(QListView::Adjust);
    _view->setModel(_model);

    auto layout = new QVBoxLayout;
    layout->addWidget(_toolbar);
    layout->addWidget(_view, 1);
    setLayout(layout);

    auto const selectionModel = _view->selectionModel();
    lazyconnect(_model, modelReset, this, selectCurrentId);
    lazyconnect(selectionModel, selectionChanged, this, viewSelectionChanged);
    lazyconnect(_view, activated, this, edit);
    lazyconnect(model->document(), reloaded, this, onReload);
}

int DataWidget::currentId() const {
    return _currentId;
}

void DataWidget::setCurrentId(int const id) {
    if (_currentId != id) {
        _currentId = id;
        updateActions();
        currentIdChanged(id);
    }
}

bool DataWidget::showEmpty() const {
    return _actions.showEmpty->isChecked();
}

DataWidget::Actions const &DataWidget::dataActions() const {
    return _actions;
}

void DataWidget::setDataActions(Actions const &actions) {
    _actions = actions;

    for (auto const act : {actions.add, actions.remove, actions.duplicate,
                           actions.imp, actions.exp, actions.edit}) {
        _toolbar->addAction(act);
    }
    _toolbar->addSeparator();
    _toolbar->addAction(actions.showEmpty);

    lazyconnect(actions.add, triggered, this, add);
    lazyconnect(actions.remove, triggered, this, remove);
    lazyconnect(actions.duplicate, triggered, this, duplicate);
    lazyconnect(actions.edit, triggered, this, edit);
    lazyconnect(actions.showEmpty, toggled, _model, setShowEmpty);
    updateActions();
}

void DataWidget::add() {
    _model->add(_currentId);
    updateActions();
}

void DataWidget::remove() {
    _model->remove(_currentId);
    updateActions();
}

void DataWidget::duplicate() {
    _model->duplicate(_currentId);
    updateActions();
}

void DataWidget::edit() {
    if (_currentId != -1) {
        editRequested(_currentId);
    }
}

void DataWidget::viewSelectionChanged(QItemSelection const &selected,
                                      QItemSelection const &deselected) {
    if (selected.size() == 1) {
        setCurrentId(_model->id(selected.indexes().first()));
    } else if (deselected.size() == 1) {
        setCurrentId(-1);
    }
}

// reason: mutates through pointer
// ReSharper disable once CppMemberFunctionMayBeConst
void DataWidget::selectCurrentId() {
    auto const model = _view->selectionModel();
    if (_currentId == -1) {
        model->clearCurrentIndex();
    } else {
        model->setCurrentIndex(_model->indexFromTable(_currentId),
                               QItemSelectionModel::SelectCurrent);
    }
}

// reason: mutates through pointer
// ReSharper disable once CppMemberFunctionMayBeConst
void DataWidget::updateActions() {
    auto const selectedExists = _model->hasId(_currentId);
    auto const canAdd = _model->canAdd();
    _actions.add->setEnabled(canAdd);
    _actions.remove->setEnabled(selectedExists);
    _actions.duplicate->setEnabled(canAdd && selectedExists);
    _actions.imp->setEnabled(canAdd);
    _actions.exp->setEnabled(selectedExists);
    _actions.edit->setEnabled(selectedExists);
}

void DataWidget::onReload() {
    setCurrentId(-1);
}