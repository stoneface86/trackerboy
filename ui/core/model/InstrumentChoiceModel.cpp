
#include "core/model/InstrumentChoiceModel.hpp"

#include "core/model/InstrumentListModel.hpp"


InstrumentChoiceModel::InstrumentChoiceModel(QObject *parent) :
    QAbstractListModel(parent),
    mModel(nullptr)
{
}

int InstrumentChoiceModel::rowCount(QModelIndex const& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    if (mModel) {
        return mModel->rowCount() + 1;
    } else {
        return 1;
    }
}

QVariant InstrumentChoiceModel::data(QModelIndex const& index, int role) const {
    auto const row = index.row();
    if (row == 0) {
        if (role == Qt::DisplayRole) {
            return tr("No instrument");
        }
    } else if (mModel) {
        return mModel->data(mModel->index(row - 1), role);
    }

    return QVariant();
}

void InstrumentChoiceModel::setModel(InstrumentListModel *model) {
    if (mModel != model) {
        if (mModel) {
            // disconnect signals from the previous model
            mModel->disconnect(this);
        }


        beginResetModel();
        mModel = model;
        endResetModel();


        if (mModel) {
            connect(mModel, &InstrumentListModel::rowsAboutToBeInserted, this, &InstrumentChoiceModel::forwardRowInsertion);
            connect(mModel, &InstrumentListModel::rowsInserted, this, &InstrumentChoiceModel::endInsertRows);
            connect(mModel, &InstrumentListModel::rowsAboutToBeRemoved, this, &InstrumentChoiceModel::forwardRowRemoval);
            connect(mModel, &InstrumentListModel::rowsRemoved, this, &InstrumentChoiceModel::endRemoveRows);
            connect(mModel, &InstrumentListModel::dataChanged, this, &InstrumentChoiceModel::forwardDataChanges);
            connect(mModel, &QObject::destroyed, this, &InstrumentChoiceModel::modelDestroyed);
        }
    }
}

void InstrumentChoiceModel::forwardRowInsertion(QModelIndex const& parent, int start, int end) {
    Q_UNUSED(parent)
    beginInsertRows(QModelIndex(), start + 1, end + 1);
}

void InstrumentChoiceModel::forwardRowRemoval(QModelIndex const& parent, int first, int last) {
    Q_UNUSED(parent)
    beginRemoveRows(QModelIndex(), first + 1, last + 1);
}

void InstrumentChoiceModel::forwardDataChanges(QModelIndex const& topLeft, QModelIndex const& bottomRight, QVector<int> const& roles) {
    emit dataChanged(
        createIndex(topLeft.row() + 1, 0, nullptr),
        createIndex(bottomRight.row() + 1, 0, nullptr),
        roles
    );
}

void InstrumentChoiceModel::modelDestroyed() {
    beginResetModel();
    mModel = nullptr;
    endResetModel();
}
