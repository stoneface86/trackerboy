
#pragma once

#include <QIdentityProxyModel>

//
// Proxy model for QAbstractListModels that prepends the string "%1# - " to its data
// where %1 is the index + 1
//
// example:
//  item1
//  blah
//  abc
//
// becomes:
//  1# - item1
//  2# - blah
//  3# - abc
//
class NumberedProxyModel : public QIdentityProxyModel {

    Q_OBJECT

public:
    using QIdentityProxyModel::QIdentityProxyModel;

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

};
