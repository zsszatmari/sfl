#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QColor>
#include <QUuid>
#include "Painter.h"

Painter::Painter(QQuickItem *parentItem, QQmlEngine *engine) : Gui::IPainter()
  , _parent(parentItem)
  , _engine(engine)
{
    Q_ASSERT(_parent);
}

Painter::~Painter()
{

}

void Painter::paint(const Gui::Color &color) const
{
    qDebug() << "painting color";
}

#pragma message("TODO")
void Painter::paint(const Gui::NamedImage &image) const
{

}

#pragma message("TODO")
void Painter::paint(const Gui::BitmapImage &image) const
{

}

void Painter::paint(const Gui::Gradient &gradient) const
{
    qDebug() << "painting gradient";

    QQmlEngine *engine = qmlEngine(_parent);
    QQmlComponent component(engine);

    component.loadUrl(QUrl("qrc:/qml/QML/GradientTemplate.qml"));
    QQuickItem *rectangle = qobject_cast<QQuickItem *>(component.create());

    auto gradientColors = gradient.colors();
    QVariantMap colorMap;
    for (auto c = gradientColors.begin(); c != gradientColors.end(); ++c)
    {
        int red = (*c).first.red() * 255;
        int green = (*c).first.green() *255;
        int blue = (*c).first.blue() * 255;
        int alpha = (*c).first.alpha() * 255;
        QColor color(red, green, blue, alpha);

        colorMap.insert(QString::number((*c).second), color.name());
    }

    if (_fillParent)
    {
       // rectangle->setProperty("anchors.fill", )
    }
    else
    {
        rectangle->setProperty("x", _leftTopX);
        rectangle->setProperty("y", _leftTopY);
        rectangle->setProperty("width", _width);
        rectangle->setProperty("height", _height);
    }
    //rectangle->setProperty("radius", _rectangleRadius);

    rectangle->setProperty("colorStops", colorMap);
    rectangle->setParentItem(_parent);
    rectangle->setProperty("z", -1);
}

