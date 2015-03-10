//
//  YoutubeProtocol.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/14/13.
//
//

#ifndef __G_Ear_Player__YoutubeProtocol__
#define __G_Ear_Player__YoutubeProtocol__

#include <string>
#include <vector>

namespace Gear
{
    using std::string;
    using std::vector;
    
    class YouTubeProtocol
    {
    public:
        // www.youtube.com/get_video_info?video_id=...
        static string videoUrl(const string &videoInfo);
        // http://www.youtube.com/watch?v=...
        static string videoUrlWeb(const string &videoInfo);
        
    private:
        YouTubeProtocol(); // delete
    };
}

#endif /* defined(__G_Ear_Player__YoutubeProtocol__) */
