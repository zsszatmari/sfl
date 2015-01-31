#include "SongListContextMenuModelItem.h"

SongListContextMenuModelItem::SongListContextMenuModelItem(QObject *parent) : ListModelItem(parent)
{

}

SongListContextMenuModelItem::~SongListContextMenuModelItem()
{

}

QVariant SongListContextMenuModelItem::data(int role) const
{
    switch (role)
    {
    case IdRole:
        return menuId();
    case MenuTextRole:
        return menuText();
    default:
        return QVariant();
    }
}

