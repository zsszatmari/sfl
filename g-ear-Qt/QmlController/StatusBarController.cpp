#include <QQmlContext>
#include "StatusBarController.h"
#include "Gui/ThemeManager.h"
#include "Gui/Style.h"
#include "Gui/Rules.h"
#include "Gui/IPaintable.h"
#include "sfl/just_ptr.h"
#include "Theme/Painter.h"

StatusBarController::StatusBarController(QQmlEngine *engine) :
    QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("statusBarController", this);
}

StatusBarController::~StatusBarController()
{

}

void StatusBarController::qmlWindowReady()
{
    QQuickItem *bottomBar = qmlWindow()->findChild<QQuickItem *>("bottomBarObjectName");
    auto paintable = Gear::IApp::instance()->themeManager()->style().get("general bottombar").paintable();
    Painter painter(bottomBar);
    painter.setFillParent(true);
    painter.setZOrder(-1);
    paintable->paint(painter);
}

