#include "core/model/NumberedProxyModel.hpp"




QVariant NumberedProxyModel::data(QModelIndex const& index, int role) const {

    if (role != Qt::DisplayRole) {
        return QIdentityProxyModel::data(index, role);
    }

    return QStringLiteral("%1# - %2").arg(
                QString::number(index.row() + 1),
                sourceModel()->data(index).toString()
                );
}