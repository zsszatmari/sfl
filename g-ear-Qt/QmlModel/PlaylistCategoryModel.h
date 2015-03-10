#ifndef PLAYLISTCATEGORYMODEL_H
#define PLAYLISTCATEGORYMODEL_H

#include "ListModel.h"

#include "IApp.h"
#include "SessionManager.h"
#include "PlaylistCategory.h"
#include "ValidPtr.h"
#include "SignalConnection.h"
#include "IPlaylist.h"

#include "PlaylistModel.h"

class QQmlEngine;

class PlaylistCategoryModel : public ListModel
{
    Q_OBJECT
public:
    explicit PlaylistCategoryModel(QQmlEngine *engine, QObject *parent = 0);

    virtual ~PlaylistCategoryModel();

    virtual QHash<int, QByteArray> roleNames() const;

private:
     Base::SignalConnection _categoryConnection;
     std::vector<std::unique_ptr<PlaylistModel>> _playlistModels;
};

#endif // PLAYLISTCATEGORYMODEL_H
