#ifndef QTPREFERENCES_H
#define QTPREFERENCES_H

#include <QSettings>
#include "IPreferences.h"

namespace Gear
{
    class QTPreferences final : public IPreferences
    {
    public:
        explicit QTPreferences();
        ~QTPreferences();
        virtual bool boolForKey(const string &key) const;
        virtual unsigned long uintForKey(const string &key) const;
        virtual void setUintForKey(const string &key, const uint64_t value);
        virtual string stringForKey(const string &key) const;
        virtual void setStringForKey(const string &key, const string &value);

        virtual float floatForKey(const string &key) const;
        virtual void setFloatForKey(const string &key, const float value);

    private:
        QSettings *_setting = nullptr;
    };
}

#endif // QTPREFERENCES_H
