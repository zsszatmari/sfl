#include "PlaylistModel.h"

PlaylistModel::PlaylistModel(QObject *parent) : ListModel(parent)
{

}

PlaylistModel::~PlaylistModel()
{
    clear();
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[PlaylistModelItem::CategoryRole] = "playlistCategoryName";
    names[PlaylistModelItem::TitleRole] = "playlistName";
    names[PlaylistModelItem::IdRole] = "playlistId";

    return names;
}

