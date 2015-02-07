#ifndef _LISTMODEL_H
#define _LISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariant>
#include <memory>
#include "ListModelItem.h"

class ListModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit ListModel(QObject* parent = 0) : QAbstractListModel(parent)
    {
    }

    virtual ~ListModel() {}

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return _itemList.size();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (index.row() < 0 || index.row() >= _itemList.size())
        {
            return QVariant();
        }
        return _itemList.at(index.row())->data(role);
    }

    void appendRow(std::unique_ptr<ListModelItem> &&item)
    {
        std::vector<std::unique_ptr<ListModelItem>> items;
        items.push_back(std::move(item));
        appendRows(std::move(items));
    }

    void appendRows(std::vector<std::unique_ptr<ListModelItem>> &&items)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount()+items.size()-1);
        for (auto &item : items)
        {
            QObject::connect(item.get(), SIGNAL(dataChanged()), SLOT(handleItemChange()));
            _itemList.push_back(std::move(item));
        }
        endInsertRows();
    }
    
    void insertRow(int row, std::unique_ptr<ListModelItem> &&item)
    {
        beginInsertRows(QModelIndex(), row, row);
        QObject::connect(item.get(), SIGNAL(dataChanged()), SLOT(handleItemChange()));
        _itemList.insert(_itemList.begin() + row, std::move(item));
        endInsertRows();
    }

    bool removeRow(int row, const QModelIndex &parent = QModelIndex())
    {
        Q_UNUSED(parent);
        if(row < 0 || row >= _itemList.size()) {
            return false;
        }

        // beginRemoveRows(QModelIndex(), row, row);
        beginResetModel();
        _itemList.erase(_itemList.begin() + row);
        endResetModel();
        // endRemoveRows();
        return true;
    }

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
    {
        Q_UNUSED(parent);
        if (row < 0 || (row+count) >= _itemList.size()) return false;
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; ++i)
        {
            _itemList.erase(_itemList.begin() + row);
        }
        endRemoveRows();
        return true;
    }

    bool removeItem(ListModelItem *item)
    {
        int row = std::distance(_itemList.begin(),
                                std::find_if(_itemList.begin(),_itemList.end(),
                                             [&](const std::unique_ptr<ListModelItem> &check){
            return check.get() == item;
        }));
        return removeRow(row);
    }
    
    ListModelItem *find(const QString &id) const
    {
        Q_UNUSED(id);
        return 0;
    }
    
    QModelIndex indexFromItem(const ListModelItem *item) const
    {
        Q_ASSERT(item);
        for (int row=0; row < _itemList.size(); ++row)
        {
            if (_itemList.at(row).get() == item) return index(row);
        }
        return QModelIndex();
    }

    void clear()
    {
        beginResetModel();
        _itemList.clear();
        endResetModel();
    }

protected:
    std::vector<std::unique_ptr<ListModelItem>> _itemList;

private slots:
    void handleItemChange()
    {
        ListModelItem *item = static_cast<ListModelItem *>(sender());
        QModelIndex index = indexFromItem(item);
        if (index.isValid())
        {
            emit dataChanged(index, index);
        }
    }
};


#endif // _LISTMODEL_H
