#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QQuickImageProvider>

class IconProvider : public QQuickImageProvider
{
public:
    explicit IconProvider();

    virtual ~IconProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    QHash<QString, QString> _iconNameHash;
};

#endif // ICONPROVIDER_H
