#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider();
    ~ImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    void addImage(const QString &imageName);
    void addImage(const QString &imageName, const uchar *data, int size);

private:
    QHash<QString, QImage> _imageHash;
};

#endif // IMAGEPROVIDER_H
