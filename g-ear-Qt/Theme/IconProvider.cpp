#include "IconProvider.h"
#include "IApp.h"
#include "IPlayer.h"
#include "Gui/ThemeManager.h"
#include "Gui/Style.h"
#include "Gui/Rules.h"
#include "Gui/IPaintable.h"
#include "sfl/just_ptr.h"
#include "Theme/Painter.h"

IconProvider::IconProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
    _iconNameHash["ShuffleNormal"] = "controls shuffle normal";
    _iconNameHash["ShuffleActive"] = "controls shuffle active";
    _iconNameHash["RepeatNormal"] = "controls repeat normal";
    _iconNameHash["RepeatActive"] = "controls repeat active";
    _iconNameHash["PlayNormal"] = "controls play normal";
    _iconNameHash["PlayPushed"] = "controls play pushed";
    _iconNameHash["PauseNormal"] = "controls pause normal";
    _iconNameHash["PausePushed"] = "controls pause pushed";
    _iconNameHash["PreNormal"] = "controls prev normal";
    _iconNameHash["PrePushed"] = "controls prev pushed";
    _iconNameHash["NextNormal"] = "controls next normal";
    _iconNameHash["NextPushed"] = "controls next pushed";
}

IconProvider::~IconProvider()
{
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);

    auto rule = Gear::IApp::instance()->themeManager()->style().get(_iconNameHash[id].toStdString());
    return QImage(QCoreApplication::applicationDirPath() + "/assets/" + QString::fromStdString(rule.imageName()) + ".png");
}
