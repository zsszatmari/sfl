#ifndef SONGLISTCONTEXTMENUMODELITEM_HPP
#define SONGLISTCONTEXTMENUMODELITEM_HPP

#include <memory>
#include "ListModelItem.h"
#include "SongListContextMenuModel.h"

class SongListContextMenuModelItem : public ListModelItem
{
    Q_OBJECT
public:
    enum Roles
    {
        IdRole,
        MenuTextRole
    };

    explicit SongListContextMenuModelItem(QObject *parent = 0);
    ~SongListContextMenuModelItem();

    virtual QVariant data(int role) const;

    inline const int &menuId() const
    {
        return _id;
    }

    inline void setMenuId(const int &id)
    {
        _id = id;
        emit dataChanged();
    }

    inline const QString &menuText() const
    {
        return _text;
    }

    inline void setMenuText(const QString &text)
    {
        _text = text;
        emit dataChanged();
    }

private:
    int _id = 0;
    QString _text = "default";
};

#endif // SONGLISTCONTEXTMENUMODELITEM_HPP
