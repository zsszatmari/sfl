#include "SongListContextMenuModel.h"

SongListContextMenuModel::SongListContextMenuModel(QObject *parent) : ListModel(parent)
{

}

SongListContextMenuModel::~SongListContextMenuModel()
{

}

QHash<int, QByteArray> SongListContextMenuModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[SongListContextMenuModelItem::IdRole] = "menuId";
    names[SongListContextMenuModelItem::MenuTextRole] = "menuText";

    return names;
}

