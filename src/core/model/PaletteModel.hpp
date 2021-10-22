
#pragma once

#include "core/Palette.hpp"

#include <QAbstractItemModel>

#include <optional>

//
// Model class for editing a Palette. Used only by the AppearanceConfigTab
//
class PaletteModel : public QAbstractItemModel {

    Q_OBJECT

public:
    explicit PaletteModel(QObject *parent = nullptr);

    Palette const& palette() const;

    void setPalette(Palette const& pal);

    virtual int columnCount(QModelIndex const& parent = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override;

    virtual bool hasChildren(QModelIndex const& index) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const override;

    virtual QModelIndex parent(QModelIndex const& index) const override;

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    virtual bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override;

    std::optional<QColor> colorAt(QModelIndex const& index) const;

    void setColor(QModelIndex const& index, QColor const& color);


private:

    Q_DISABLE_COPY(PaletteModel)

    void updateColor(QModelIndex const& index, Palette::Color paletteIndex, QColor const& color);


    Palette mPalette;


};
