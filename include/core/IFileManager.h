//
//  IFileManager.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__IFileManager__
#define __G_Ear_Player__IFileManager__

#include <vector>
#include <string>
#include "Environment.h"

namespace Gear
{
    using std::vector;
    using std::string;
    
	class core_export IFileManager
    {
    public:
        virtual ~IFileManager();
        virtual void removeFilesIfExceedsCache(const string &dir, const long long maxSize) = 0;
        virtual vector<char> getFileWithBasename(const string &dir, const string &basename) const = 0;
        virtual vector<string> listFiles(const string &dir) = 0;
        virtual void deleteFileWithBasename(const string &dir, const string &basename) = 0;
        virtual void deleteFile(const string &file) = 0;
        virtual void putFile(const string &dir, const string &filename, const vector<char> &data) = 0;
    
        virtual void getImageDimensions(const vector<char> &data, long &width, long &height) = 0;
    };
}

#endif /* defined(__G_Ear_Player__IFileManager__) */
