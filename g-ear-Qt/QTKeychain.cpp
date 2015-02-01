#include <QSettings>
#include <QCoreApplication>
#include <string>
#include <iostream>
#include <QDataStream>
#include <sstream>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

#include "QTKeychain.h"

namespace Gear
{
    #define method QTKeychain::

    QDataStream& operator<<(QDataStream &out, const UserAccount &v)
    {
        out << v.username << v.password;
        return out;
    }

    QDataStream& operator>>(QDataStream &in, UserAccount &v)
    {
        in >> v.username;
        in >> v.password;
        return in;
    }

    class EncryptTool
    {
    public:
        static std::string encryptPassword(string password);
        static string decryptPassword(std::string encryptedPassword);

    private:
    #ifdef _WIN32
        static std::string hex2Asc(const BYTE *hex, int len);
        static int hexStringToByte(LPCSTR lpHex, LPBYTE *ppByte);
    #endif
    };

    method QTKeychain()
    {
        _setting = new QSettings(QCoreApplication::applicationDirPath()
                                 + "/Setting/keychain.ini",
                                 QSettings::IniFormat);
        qRegisterMetaTypeStreamOperators<UserAccount>("UserAccount");
    }

    method ~QTKeychain()
    {
        if (_setting)
        {
            delete _setting;
            _setting = nullptr;
        }
    }

    string method username(const string &identifier) const
    {
        Q_ASSERT(_setting);

        QVariant v = _setting->value(QString::fromStdString(identifier));
        if (v.canConvert<UserAccount>())
        {
            UserAccount account = v.value<UserAccount>();
            return account.username.toStdString();
        }

        return "";
    }

    string method pass(const string &identifier) const
    {
        Q_ASSERT(_setting);

        QVariant v = _setting->value(QString::fromStdString(identifier));
        if (v.canConvert<UserAccount>())
        {
            UserAccount account = v.value<UserAccount>();
            return EncryptTool::decryptPassword(account.password.toStdString());
        }

        return "";
    }

    void method save(const string &identifier, const string &username, const string &password)
    {
        Q_ASSERT(_setting);

        UserAccount account;
        account.username = QString::fromStdString(username);
        account.password = QString::fromStdString(EncryptTool::encryptPassword(password));

        QVariant v;
        v.setValue<UserAccount>(account);
        _setting->setValue(QString::fromStdString(identifier), v);
    }

    void method forget(const string &identifier)
    {
        Q_ASSERT(_setting);

        _setting->remove(QString::fromStdString(identifier));
    }

    std::string EncryptTool::encryptPassword(std::string password)
    {
        std::string data;

    #ifdef _WIN32
        DATA_BLOB DataIn;
        DATA_BLOB DataOut;
        BYTE *pbDataInput =(BYTE *)password.c_str();
        DWORD cbDataInput = strlen((char *)pbDataInput);
        DataIn.pbData = pbDataInput;
        DataIn.cbData = cbDataInput;

        //printf("The data to be encrypted is: %s\n", pbDataInput);

        if (CryptProtectData(
                    &DataIn,
                    L"This is the description string.", // A description string.
                    nullptr,                               // Optional entropy
                    // not used.
                    nullptr,                               // Reserved.
                    nullptr,                      // Pass a PromptStruct.
                    0,
                    &DataOut))
        {
            //std::cout << "The encryption phase worked." << std::endl;
            data = hex2Asc(DataOut.pbData, DataOut.cbData);
            LocalFree(DataOut.pbData);
        }
        else
        {
            std::cout << "Encryption error!" << std::endl;
        }
        return data;
    #else
        return password;
    #endif
    }

    std::string EncryptTool::decryptPassword(std::string encryptedPassword)
    {
    #ifdef _WIN32
        LPBYTE lpByte = NULL;
        int nLen = hexStringToByte(encryptedPassword.c_str(), &lpByte);
        DATA_BLOB DataVerify;
        LPWSTR pDescrOut = nullptr;
        DATA_BLOB DataOut;
        DataOut.cbData = nLen;
        DataOut.pbData = lpByte;
        if (CryptUnprotectData(
                    &DataOut,
                    &pDescrOut,
                    nullptr,                 // Optional entropy
                    nullptr,                 // Reserved
                    nullptr,        // Optional PromptStruct
                    0,
                    &DataVerify))
        {
            LPSTR lpStrOut(new char[DataVerify.cbData + 1]);
            memcpy(lpStrOut, DataVerify.pbData, DataVerify.cbData);
            lpStrOut[DataVerify.cbData] = 0;
            LocalFree(DataVerify.pbData);

            if (pDescrOut)
            {
                LocalFree((HLOCAL)pDescrOut);
            }

            if (lpByte)
            {
                delete[] lpByte;
            }

            std::string strRet = lpStrOut;
            delete[] lpStrOut;

#ifdef DEBUG
            //std::cout << "decrypted data: '" << strRet << "'" << std::endl;
#endif
            return strRet;
        }
        else
        {
            std::cout << "Decryption error!" << std::endl;
            return "";
        }
    #else
        return encryptedPassword;
    #endif
    }

    #ifdef _WIN32
    std::string EncryptTool::hex2Asc(const BYTE *hex, int len)
    {
        static char asc[4096 * 2];
        int i;

        for (i = 0; i < len; i++)
        {
            asc[i * 2] = "0123456789ABCDEF"[hex[i] >> 4];
            asc[i * 2 + 1] = "0123456789ABCDEF"[hex[i] & 0x0F];
        }
        asc[i * 2] = '\0';

        return asc;
    }

    int EncryptTool::hexStringToByte(LPCSTR lpHex, LPBYTE *ppByte)
    {
        std::string straHex = lpHex;
        int nHexLen = straHex.size();

        LPBYTE lpByte = new byte[nHexLen / 2];
        int nCount = 0;
        for (int i = 0; i < nHexLen / 2; i++)
        {
            std::stringstream ss;
            DWORD bb;
            ss << straHex.at(i * 2);
            ss << straHex.at(i * 2 + 1);
            ss << '\0';
            ss >> std::hex >> bb;
            lpByte[i] = bb;
            nCount++;
        }
        *ppByte = lpByte;
        return nCount;
    }

    #endif

}
