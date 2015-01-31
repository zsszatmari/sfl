#ifndef SONGLISTCONTEXTMENUMODEL_HPP
#define SONGLISTCONTEXTMENUMODEL_HPP

#include "ListModel.h"
#include "SongListContextMenuModelItem.h"

class SongListContextMenuModel : public ListModel
{
    Q_OBJECT
public:
    explicit SongListContextMenuModel(QObject *parent = 0);
    ~SongListContextMenuModel();

    virtual QHash<int, QByteArray> roleNames() const;
};

#endif // SONGLISTCONTEXTMENUMODEL_HPP
