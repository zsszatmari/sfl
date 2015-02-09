#include "PlaylistCategoryModelItem.h"

PlaylistCategoryModelItem::PlaylistCategoryModelItem(QObject *parent) :
    ListModelItem(parent)
{
}

PlaylistCategoryModelItem::~PlaylistCategoryModelItem()
{

}

QVariant PlaylistCategoryModelItem::data(int role) const
{
    switch (role)
    {
    case CategoryRole:
        return categoryName();
    case PlaylistModelRole:
        return playlistModel();
    case PlaylistCountRole:
        return playlistCount();
    default:
        return QVariant();
    }
}
