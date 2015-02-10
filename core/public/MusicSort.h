//
//  MusicSort.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/28/13.
//
//

#ifndef G_Ear_Player_MusicSort_h
#define G_Ear_Player_MusicSort_h

#include <string>

namespace Gear
{
    using std::string;

    class MusicSort final
    {
    public:    
        static const std::string strip(const string &str);
        template<class S>
        static int stripOffset(const S &str);
        static const std::string toStrip[];
        static const int toStripCount;
        
    private:
    };
    
#define method MusicSort::

    inline const string method strip(const string &str)
    {        
        for (int i = 0 ; i < toStripCount  ; ++i) {
            if (str.compare(0, toStrip[i].size(), toStrip[i]) == 0) {
                return str.substr(toStrip[i].size());
            }
        }
        return str;
    }
    
    template<class S>
    int method stripOffset(const S &str)
    {
        for (int i = 0 ; i < toStripCount ; ++i) {
            if (str.compare(0, toStrip[i].size(), toStrip[i]) == 0) {
                return toStrip[i].size();
            }
        }
        return 0;
    }
    
#undef method
}

#endif
