#include "MusicSort.h"

namespace Gear
{
    const std::string MusicSort::toStrip[] = {" ", "the ", "The "};
    //static const string toStrip[] = {" ", "an ", "a ", "the ", "An ", "A ", "The "};
    const int MusicSort::toStripCount = sizeof(MusicSort::toStrip)/sizeof(std::string);
}