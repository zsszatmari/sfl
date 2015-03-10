#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QColor>
#include <QUuid>
#include "Painter.h"

Painter::Painter(QQuickItem *parentItem) : Gui::IPainter()
  , _parent(parentItem)
{
    Q_ASSERT(_parent);
}

Painter::~Painter()
{

}

void Painter::paint(const Gui::Color &color) const
{
    QQmlEngine *engine = qmlEngine(_parent);
    QQmlComponent component(engine);

    component.loadUrl(QUrl("qrc:/qml/QML/ColorTemplate.qml"));
    QQuickItem *rectangle = qobject_cast<QQuickItem *>(component.create());

    int red = color.red() * 255;
    int green = color.green() *255;
    int blue = color.blue() * 255;
    int alpha = color.alpha() * 255;
    QColor bgColor(red, green, blue, alpha);

    if (_fillParent)
    {
        rectangle->setProperty("fillParent", true);
    }
    else
    {
        rectangle->setProperty("fillParent", false);
        rectangle->setProperty("x", _leftTopX);
        rectangle->setProperty("y", _leftTopY);
        rectangle->setProperty("width", _width);
        rectangle->setProperty("height", _height);
    }
    rectangle->setProperty("radius", _rectangleRadius);

    rectangle->setProperty("color", bgColor);
    rectangle->setParentItem(_parent);
    rectangle->setProperty("z", _zOrder);
}

void Painter::paint(const Gui::NamedImage &image) const
{
    if (_action == Painter::AddImageAction)
    {
        QQmlEngine *engine = qmlEngine(_parent);
        QQmlComponent component(engine);

        component.loadUrl(QUrl("qrc:/qml/QML/ImageTemplate.qml"));
        QQuickItem *imageItem = qobject_cast<QQuickItem *>(component.create());

        if (_fillParent)
        {
            imageItem->setProperty("fillParent", true);
        }
        else
        {
            imageItem->setProperty("fillParent", false);
            imageItem->setProperty("x", _leftTopX);
            imageItem->setProperty("y", _leftTopY);
            imageItem->setProperty("width", _width);
            imageItem->setProperty("height", _height);
        }
        _imageProvider->addImage(QString::fromStdString(image.imageName()));
        imageItem->setProperty("source", "image://"
                               + _imageProviderName
                               + "/" + QString::fromStdString((image.imageName())));
        imageItem->setParentItem(_parent);
        imageItem->setProperty("z", _zOrder);
    }
    else if (_action == Painter::SetImageAction)
    {
        _imageProvider->addImage(QString::fromStdString(image.imageName()));
        _parent->setProperty("source", "image://"
                             + _imageProviderName
                             + "/" + QString::fromStdString((image.imageName())));
        _parent->setProperty("z", _zOrder);
    }
}

void Painter::paint(const Gui::BitmapImage &image) const
{
    if (_action == Painter::AddImageAction)
    {
        QQmlEngine *engine = qmlEngine(_parent);
        QQmlComponent component(engine);

        component.loadUrl(QUrl("qrc:/qml/QML/ImageTemplate.qml"));
        QQuickItem *imageItem = qobject_cast<QQuickItem *>(component.create());

        if (_fillParent)
        {
            imageItem->setProperty("fillParent", true);
        }
        else
        {
            imageItem->setProperty("fillParent", false);
            imageItem->setProperty("x", _leftTopX);
            imageItem->setProperty("y", _leftTopY);
            imageItem->setProperty("width", _width);
            imageItem->setProperty("height", _height);
        }
        _imageProvider->addImage(QString::fromStdString(image.imageName()),
                                 (unsigned char *)&(image.data().data()[0]),
                image.data().size());
        imageItem->setProperty("source", "image://"
                               + _imageProviderName
                               + "/" + QString::fromStdString((image.imageName())));
        imageItem->setParentItem(_parent);
        imageItem->setProperty("z", _zOrder);
    }
    else if (_action == Painter::SetImageAction)
    {
        _imageProvider->addImage(QString::fromStdString(image.imageName()),
                                 (unsigned char *)&(image.data().data()[0]),
                image.data().size());
        _parent->setProperty("source", "image://"
                             + _imageProviderName
                             + "/" + QString::fromStdString((image.imageName())));
        _parent->setProperty("z", _zOrder);
    }
}

void Painter::paint(const Gui::Gradient &gradient) const
{
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
        rectangle->setProperty("fillParent", true);
    }
    else
    {
        rectangle->setProperty("fillParent", false);
        rectangle->setProperty("x", _leftTopX);
        rectangle->setProperty("y", _leftTopY);
        rectangle->setProperty("width", _width);
        rectangle->setProperty("height", _height);
    }

    rectangle->setProperty("colorStops", colorMap);
    rectangle->setParentItem(_parent);
    rectangle->setProperty("z", _zOrder);
}

