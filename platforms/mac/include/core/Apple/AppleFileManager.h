//
//  AppleFileManager.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__AppleFileManager__
#define __G_Ear_Player__AppleFileManager__

#include "IFileManager.h"

namespace Gear
{
    class AppleFileManager final : public IFileManager
    {
    public:
        AppleFileManager();
        
        virtual void removeFilesIfExceedsCache(const string &dir, const long long maxSize);
        virtual vector<char> getFileWithBasename(const string &dir, const string &basename) const;
        virtual void deleteFileWithBasename(const string &dir, const string &basename);
        virtual void deleteFile(const string &file);
        virtual void putFile(const string &dir, const string &filename, const vector<char> &data);
        
        virtual void getImageDimensions(const vector<char> &data, long &width, long &height);
        virtual vector<string> listFiles(const string &dir);
        
    private:
        NSMutableArray *dirEntries(NSURL *url) const;
    };
}

#endif /* defined(__G_Ear_Player__AppleFileManager__) */
