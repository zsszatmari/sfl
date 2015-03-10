#ifndef QTKEYCHAIN_H
#define QTKEYCHAIN_H

#include <QSettings>
#include "IKeychain.h"

namespace Gear
{
    struct UserAccount
    {
        QString username;
        QString password;
    };
}

Q_DECLARE_METATYPE(Gear::UserAccount)

namespace Gear
{
    class QTKeychain final : public IKeychain
    {
    public:
        QTKeychain();
        ~QTKeychain();
        virtual string username(const string &identifier) const;
        virtual string pass(const string &identifier) const;
        virtual void save(const string &identifier, const string &username, const string &password);
        virtual void forget(const string &identifier);

    private:
        QSettings *_setting = nullptr;
    };

}
#endif // QTKEYCHAIN_H
