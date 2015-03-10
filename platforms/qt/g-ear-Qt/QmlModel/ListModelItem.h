#ifndef _LISTMODELITEM_H
#define _LISTMODELITEM_H

#include <QObject>

class ListModelItem : public QObject
{
    Q_OBJECT
public:
    explicit ListModelItem(QObject *parent = 0) : QObject(parent)
    {

    }
    virtual ~ListModelItem()
    {

    }
    virtual QVariant data(int role) const = 0;

signals:
    void dataChanged();
};


#endif // _LISTMODELITEM_H
