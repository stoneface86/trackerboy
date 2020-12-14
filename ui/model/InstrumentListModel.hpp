
#pragma once

#include "model/BaseTableModel.hpp"

#include <array>
#include <QIcon>

class InstrumentListModel : public BaseTableModel {

    Q_OBJECT

public:
    InstrumentListModel(ModuleDocument &document);

    // get the instrument associated with the model index
    trackerboy::Instrument* instrument(int modelIndex) const;

    // set the current instrument's channel
    void setChannel(trackerboy::ChType ch);

    void setPanning(uint8_t panning);

    void setDelay(uint8_t delay);

    void setDuration(uint8_t duration);

    void setTune(int8_t tune);

    void setVibrato(uint8_t extent, uint8_t speed);

    void setVibratoDelay(uint8_t delay);

    void setEnvelope(uint8_t envelope);

    void setTimbre(uint8_t timbre);


protected:
    QVariant iconData(const QModelIndex &index) const override;

private:

    std::array<QIcon*, 4> mIconArray;

};
