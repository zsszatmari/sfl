#include <QCoreApplication>
#include <QDebug>

#include "QTPreferences.h"

namespace Gear
{

#define method QTPreferences::

    method QTPreferences()
    {
        _setting = new QSettings(QCoreApplication::applicationDirPath()
                                 + "/Setting/preferences.ini",
                                 QSettings::IniFormat);
    }

    method ~QTPreferences()
    {
        if (_setting)
        {
            delete _setting;
            _setting = nullptr;
        }
    }

    bool method boolForKey(const string &key) const
    {
        Q_ASSERT(_setting);

        return _setting->value(QString::fromStdString(key), false).toBool();
    }

    unsigned long method uintForKey(const string &key) const
    {
        Q_ASSERT(_setting);

        return _setting->value(QString::fromStdString(key)).toULongLong();
    }

    void method setUintForKey(const string &key, const uint64_t value)
    {
        Q_ASSERT(_setting);

        _setting->setValue(QString::fromStdString(key), value);
    }

    std::string method stringForKey(const std::string &key) const
    {
        Q_ASSERT(_setting);

        return _setting->value(QString::fromStdString(key)).toString().toStdString();
    }

    void method setStringForKey(const std::string &key, const std::string &value)
    {
        Q_ASSERT(_setting);

        _setting->setValue(QString::fromStdString(key), QString::fromStdString(value));
    }

    float method floatForKey(const string &key) const
    {
        return _setting->value(QString::fromStdString(key)).toFloat();
    }

    void method setFloatForKey(const string &key, const float value)
    {
        _setting->setValue(QString::fromStdString(key), value);
    }

}
