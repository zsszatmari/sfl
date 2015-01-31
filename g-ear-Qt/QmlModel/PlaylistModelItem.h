#ifndef _PLAYLISTMODELITEM_H
#define _PLAYLISTMODELITEM_H

#include "ListModelItem.h"

class PlaylistModelItem : public ListModelItem
{
    Q_OBJECT
public:
    enum Roles
    {
        CategoryRole,
        TitleRole,
        IdRole
    };

    explicit PlaylistModelItem(QObject *parent = 0);

    ~PlaylistModelItem();

    virtual QVariant data(int role) const;

    inline const QString &categoryName() const
    {
        return _categoryName;
    }

    inline void setCategoryName(const QString &name)
    {
        _categoryName = name;
        emit dataChanged();
    }

    inline const QString &title() const
    {
        return _title;
    }

    inline void setTitle(const QString &title)
    {
        _title = title;
        emit dataChanged();
    }

    inline const QString &id() const
    {
        return _id;
    }

    inline void setId(const QString &id)
    {
        _id = id;
        emit dataChanged();
    }

private:
    QString _categoryName = "";
    QString _title = "";
    QString _id = "";
};


#endif // _PLAYLISTMODELITEM_H
