#include <QVariant>
#include "PlaylistModelItem.h"

PlaylistModelItem::PlaylistModelItem(QObject *parent) : ListModelItem(parent)
{
}

PlaylistModelItem::~PlaylistModelItem()
{
}

QVariant PlaylistModelItem::data(int role) const
{
    switch (role)
    {
    case CategoryRole:
        return categoryName();
    case TitleRole:
        return title();
    case IdRole:
        return id();
    default:
        return QVariant();
    }
}
