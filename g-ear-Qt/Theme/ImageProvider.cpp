#include <QCoreApplication>
#include "ImageProvider.h"

ImageProvider::ImageProvider() : QQuickImageProvider(QQmlImageProviderBase::Image)
{

}

ImageProvider::~ImageProvider()
{

}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);
    return _imageHash[id];
}

void ImageProvider::addImage(const QString &imageName)
{
    if (_imageHash.contains(imageName))
    {
        _imageHash.remove(imageName);
    }

    _imageHash.insert(imageName, QImage(QCoreApplication::applicationDirPath() + "/assets/" + imageName + ".png"));
}

void ImageProvider::addImage(const QString &imageName, const uchar *data, int size)
{
    if (_imageHash.contains(imageName))
    {
        _imageHash.remove(imageName);
    }

    _imageHash.insert(imageName, QImage::fromData(data, size));
}

