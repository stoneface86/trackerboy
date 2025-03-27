
#pragma once

#include "utils/aliases.hxx"

#include <QObject>

class GraphModel : public QObject {

    Q_OBJECT

public:
    explicit GraphModel(QObject *parent = nullptr);

    int len() const;

    i8 get(int idx) const;

    void set(int idx, i8 data);

    void setSource(i8 *data, int len);

signals:
    void sourceChanged(int len);
    void dataChanged();

private:
    i8 *mDataSource;
    int mDataSourceLen;


};