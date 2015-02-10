//
//  GooglePlaySignature.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/26/13.
//
//

#ifndef __G_Ear_Player__GooglePlaySignature__
#define __G_Ear_Player__GooglePlaySignature__

#include <string>

namespace Gear
{
    using std::string;
    
    class GooglePlaySignature
    {
    public:
        GooglePlaySignature();
        
        const string generateForSongId(const string &songId) const;
        const string &salt() const;
        void setSalt(const string &salt);
        
    private:
        string _salt;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlaySignature__) */
