#ifndef QTFILEMANAGER_H
#define QTFILEMANAGER_H

#include "IFileManager.h"

using std::string;
using std::vector;

class BoostFileManager : public Gear::IFileManager
{
public:
    virtual void removeFilesIfExceedsCache(const string &dir, const long long maxSize);
    virtual vector<char> getFileWithBasename(const string &dir, const string &basename) const;
    virtual vector<string> listFiles(const string &dir);
    virtual void deleteFileWithBasename(const string &dir, const string &basename);
    virtual void deleteFile(const string &file);
    virtual void putFile(const string &dir, const string &filename, const vector<char> &data);

    virtual void getImageDimensions(const vector<char> &data, long &width, long &height);
};

#endif // QTFILEMANAGER_H
