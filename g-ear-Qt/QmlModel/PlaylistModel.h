#ifndef _PLAYLISTMODEL_H
#define _PLAYLISTMODEL_H

#include "ListModel.h"
#include "PlaylistModelItem.h"

#include "IApp.h"
#include "SessionManager.h"
#include "PlaylistCategory.h"
#include "ValidPtr.h"

class PlaylistModel : public ListModel
{
    Q_OBJECT
public:
    explicit PlaylistModel(QObject *parent = 0);

    ~PlaylistModel();
    
    virtual QHash<int, QByteArray> roleNames() const;
};

//Q_DECLARE_METATYPE(PlaylistModel)

#endif // _PLAYLISTMODEL_H
