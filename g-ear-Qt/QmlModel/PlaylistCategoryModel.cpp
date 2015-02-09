#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>

#include "PlaylistCategoryModel.h"
#include "PlaylistCategoryModelItem.h"
#include "PlaylistModel.h"
#include "PlaylistModelItem.h"

PlaylistCategoryModel::PlaylistCategoryModel(QQmlEngine *engine, QObject *parent) :
    ListModel(parent)
{
    Q_ASSERT(engine);

    _categoryConnection = Gear::IApp::instance()->sessionManager()->playlistsChangeEvent()
            .connect([=]()
    {
        this->beginResetModel();
        _playlistModels.clear();
        this->clear();
        auto categories = Gear::IApp::instance()->sessionManager()->categories();
        for (auto it = categories->begin(); it != categories->end(); ++it)
        {
            std::unique_ptr<PlaylistCategoryModelItem> categoryModelItem(new PlaylistCategoryModelItem(this));
            auto playlistCategory = (*it);
            categoryModelItem->setCategoryName(QString::fromStdString(playlistCategory.title()));

            auto playlist = playlistCategory.playlists();
            categoryModelItem->setPlaylistCount(playlist.size());

            std::unique_ptr<PlaylistModel> playlistModel(new PlaylistModel());
            for (auto i = playlist.begin(); i != playlist.end(); ++i)
            {
                auto p = (*i);
                std::unique_ptr<PlaylistModelItem> item(new PlaylistModelItem());
                item->setCategoryName(categoryModelItem->categoryName());
                item->setTitle(QString::fromStdString(p->name()));
                item->setId(QString::fromStdString(p->playlistId()));
                playlistModel->appendRow(std::move(item));
            }

            QString modelName = QString::fromStdString(playlistCategory.title()) + "Model";
            engine->rootContext()->setContextProperty(modelName, playlistModel.get());

            categoryModelItem->setPlaylistModel(engine->rootContext()->contextProperty(modelName));

            this->appendRow(std::move(categoryModelItem));

            _playlistModels.push_back(std::move(playlistModel));
        }
        this->endResetModel();
    });
}

PlaylistCategoryModel::~PlaylistCategoryModel()
{
    clear();
}

QHash<int, QByteArray> PlaylistCategoryModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[PlaylistCategoryModelItem::CategoryRole] = "categoryName";
    names[PlaylistCategoryModelItem::PlaylistModelRole] = "playlistModel";
    names[PlaylistCategoryModelItem::PlaylistCountRole] = "playlistCount";

    return names;
}
