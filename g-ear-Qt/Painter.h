#ifndef PAINTER_HPP
#define PAINTER_HPP

#include <QQuickItem>
#include <memory>
#include "IApp.h"
#include "Gui/IPaintable.h"
#include "Gui/IPainter.h"
#include "Gui/Color.h"
#include "Gui/BitmapImage.h"
#include "Gui/Gradient.h"

class Painter : public Gui::IPainter
{
public:
    Painter(QQuickItem *parentItem);
    virtual ~Painter();

    virtual void paint(const Gui::Color &color) const;
    virtual void paint(const Gui::NamedImage &image) const;
    virtual void paint(const Gui::BitmapImage &image) const;
    virtual void paint(const Gui::Gradient &gradient) const;

    inline void setZOrder(const int &z)
    {
        _zOrder = z;
    }

    inline void setLeftTopX(const int &x)
    {
        _leftTopX = x;
    }

    inline void setLeftTopY(const int &y)
    {
        _leftTopY = y;
    }

    inline void setWidth(const int &width)
    {
        _width =  width;
    }

    inline void setHeight(const int &height)
    {
        _height = height;
    }

    inline void setRectangleRadius(const int &radius)
    {
        _rectangleRadius = radius;
    }

    inline void setBorderLineWidth(const int &width)
    {
        _borderLineWidth = width;
    }

    inline void setObjectName(const QString &name)
    {
        _objectName = name;
    }

    inline void setFillParent(const bool &fillParent)
    {
        _fillParent = fillParent;
    }

private:
    int _zOrder = 0;
    int _leftTopX = 0;
    int _leftTopY = 0;
    int _width = 0;
    int _height = 0;
    int _rectangleRadius = 0;
    int _borderLineWidth = 0;
    bool _fillParent = false;
    QString _objectName = "";
    QQuickItem *_parent = nullptr;
};

#endif // PAINTER_HPP
