#ifndef PLAYLISTCATEGORYMODELITEM_H
#define PLAYLISTCATEGORYMODELITEM_H

#include "ListModelItem.h"
#include "PlaylistModel.h"

class PlaylistCategoryModelItem : public ListModelItem
{
    Q_OBJECT
public:
    enum Roles
    {
        CategoryRole,
        PlaylistModelRole,
        PlaylistCountRole
    };

    explicit PlaylistCategoryModelItem(QObject *parent = 0);

    ~PlaylistCategoryModelItem();

    virtual QVariant data(int role) const;

    inline const QString &categoryName() const
    {
        return _name;
    }

    inline void setCategoryName(const QString &name)
    {
        _name = name;
        emit dataChanged();
    }

    inline const QVariant &playlistModel() const
    {
        return _playlistModel;
    }

    inline void setPlaylistModel(const QVariant &model)
    {
        _playlistModel = model;
        emit dataChanged();
    }

    inline const int &playlistCount() const
    {
        return _playlistCount;
    }

    inline void setPlaylistCount(const int &count)
    {
        _playlistCount = count;
    }

private:
    QString _name = "";
    QVariant _playlistModel = QVariant();
    int _playlistCount = 0;
};

#endif // PLAYLISTCATEGORYMODELITEM_H
