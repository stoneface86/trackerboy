
#include "core/model/TableModel.hpp"
#include "core/IconManager.hpp"

#include <type_traits>


template <>
TableModel<trackerboy::Instrument>::TableModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, tr("New instrument"), parent)
{
}

template <>
TableModel<trackerboy::Waveform>::TableModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, tr("New waveform"), parent)
{
}

template <class T>
std::shared_ptr<T> TableModel<T>::getShared(int index) {
    if (index == -1) {
        return nullptr;
    } else {
        return source().getShared(id(index));
    }

}

template <class T>
QIcon TableModel<T>::iconData(int id) const {
    if constexpr (std::is_same_v<T, trackerboy::Instrument>) {
        auto ch = source().get(id)->channel();
        Icons icons;
        switch (ch) {
            case trackerboy::ChType::ch1:
                icons = Icons::ch1;
                break;
            case trackerboy::ChType::ch2:
                icons = Icons::ch2;
                break;
            case trackerboy::ChType::ch3:
                icons = Icons::ch3;
                break;
            default:
                icons = Icons::ch4;
                break;

        }

        return IconManager::getIcon(icons);
    } else {
        // no icons for waveforms (yet, potential TODO)
        Q_UNUSED(id);
        return {};
    }
}

template <class T>
void TableModel<T>::commitName(int id, std::string &&name) {
    auto item = source()[id];
    Q_ASSERT(item != nullptr);
    item->setName(std::move(name));
}

template <class T>
std::string const* TableModel<T>::sourceName(int id) {
    auto item = source()[id];
    if (item == nullptr) {
        return nullptr;
    } else {
        return &item->name();
    }
}

template <class T>
int TableModel<T>::sourceAdd() {
    auto &_source = source();
    auto id = _source.nextAvailableId();

    auto item = _source.insert();
    Q_ASSERT(item != nullptr);
    (void)item;

    return id;
}

template <class T>
int TableModel<T>::sourceDuplicate(int dupedId) {
    auto &_source = source();
    auto id = _source.nextAvailableId();

    auto item = _source.duplicate(dupedId);
    Q_ASSERT(item != nullptr);
    (void)item;

    return id;
}

template <class T>
void TableModel<T>::sourceRemove(int id) {
    source().remove(id);
}

template <class T>
trackerboy::Table<T> const&
TableModel<T>::source() const {
    if constexpr (std::is_same_v<T, trackerboy::Instrument>) {
        return mModule.data().instrumentTable();
    } else {
        return mModule.data().waveformTable();
    }
}

template <class T>
trackerboy::Table<T>&
TableModel<T>::source() {
    return const_cast<trackerboy::Table<T>&>(
                static_cast<TableModel<T> const&>(*this).source()
                );
}

template class TableModel<trackerboy::Instrument>;
template class TableModel<trackerboy::Waveform>;
