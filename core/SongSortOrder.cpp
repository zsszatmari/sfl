//
//  SongSortOrder.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/15/13.
//
//

#include <map>
#include "SongSortOrder.h"
#include "BaseUtility.h"
#include "AppBase.h"
#include "IPreferences.h"
#include "SongEntry.h"
#include "MusicSortCompare.h"
#include "Logger.h"

namespace Gear
{
#define method SongSortOrder::
    
    using std::map;
    using std::make_pair;
    
    vector<string> method defaultSortOrder()
    {
        /*if (IApp::instance()->phoneInterface()) {
            string s[] = {"relevance", "position", "title", "artist", "year", "album", "disc", "track"};
            return Base::init<vector<string>>(s);
        } else {*/
            string s[] = {"relevance", "position", "artist", "year", "album", "disc", "track", "title"};
            return Base::init<vector<string>>(s);
        //}
    }
    
    SortDescriptor method sortDescriptor(const string &key, bool ascending)
    {
        string protoKey = key;
        
        map<string, string> normedKeys;
        // don't use norm anymore for sorting
        /*normedKeys.insert(make_pair("artist", "artistNorm"));
        normedKeys.insert(make_pair("album", "albumNorm"));
        normedKeys.insert(make_pair("title", "titleNorm"));
        */

        auto sortOrder = defaultSortOrder();
        
        {
            auto it = normedKeys.find(protoKey);
            if (it != normedKeys.end()) {
                protoKey = it->second;
            }
        }
        
        std::transform(sortOrder.begin(), sortOrder.end(), sortOrder.begin(), [&normedKeys](const string &lhs)-> string{
            auto it = normedKeys.find(lhs);
            if (it != normedKeys.end()) {
                return it->second;
            }
            return lhs;
        });
        
        {
            auto it = find(sortOrder.begin(), sortOrder.end(), protoKey);
            if (it != sortOrder.end()) {
                sortOrder.erase(it);
            }
            
            if (!protoKey.empty()) {
                // compilation albums...
                if (protoKey == "disc") {
                    auto it = find(sortOrder.begin(), sortOrder.end(), "track");
                    if (it != sortOrder.end()) {
                        sortOrder.erase(it);
                    }
                    sortOrder.insert(sortOrder.begin(), "track");
                }
                sortOrder.insert(sortOrder.begin(), protoKey);
            }
        }
    
        if (!protoKey.empty() && protoKey != "position" && protoKey != "relevance") {
            sortOrder.erase(remove(sortOrder.begin(), sortOrder.end(), "position"), sortOrder.end());
            sortOrder.erase(remove(sortOrder.begin(), sortOrder.end(), "relevance"), sortOrder.end());
        }
        bool forceSortByAlbumName = IApp::instance()->preferences().boolForKey("ForceSortByAlbumName");
        if (forceSortByAlbumName) {
            if (protoKey != "year") {
                sortOrder.erase(remove(sortOrder.begin(), sortOrder.end(), "year"), sortOrder.end());
            }
        }
        
        if (protoKey == "album"/*"albumNorm"*/) {
            // think about the compilation albums
            sortOrder.erase(remove(sortOrder.begin(), sortOrder.end(), "artist"/*"artistNorm"*/), sortOrder.end());
        }
        
        
        vector<pair<string, bool>> keysAndAscending;
        
        auto addToFinal = [&](const std::string &key){
            keysAndAscending.push_back(make_pair(key, ascending));
            ascending = true;
        };
        for (auto it = sortOrder.begin() ; it != sortOrder.end() ; ++it) {
            auto useKey = *it;
            
            addToFinal(useKey);
            
            if (!forceSortByAlbumName) {
                // handle similar named, but in reality different albums
                if (useKey == "artist"/*"artistNorm"*/) {
                    addToFinal("artistId");
                } else if (useKey == "album"/*"albumNorm"*/) {
                    addToFinal("albumId");
                }
            }
        }
        return SortDescriptor(keysAndAscending);
    }
    
    function<bool(const SongEntry &,const SongEntry &)> method comparator(const SortDescriptor &sortDescriptor)
    {
        vector<function<bool(const shared_ptr<ISong>,const shared_ptr<ISong>)>> comps;
        comps.reserve(sortDescriptor.keysAndAscending().size());
        auto &keys = sortDescriptor.keysAndAscending();
        
        for (auto it = keys.begin() ; it != keys.end() ; ++it) {
            
            auto &p = *it;
            const auto key = p.first;
            const auto asc = p.second;
            
            function<bool(const shared_ptr<ISong>,const shared_ptr<ISong>)> comp;
            if (key == "position") {
                comp = [](const shared_ptr<ISong> &lhs, const shared_ptr<ISong> &rhs){
                    
                    // handled later, just treat them if they were the same
                    return false;
                };
            } else if (key == "durationMillis" || key == "track" || key == "year" || key == "playCount" || key == "lastPlayed" || key == "creationDate" || key == "rating" || key == "relevance") {
                if (asc) {
                    // ascending
                    comp = [=](const shared_ptr<ISong> lhs, const shared_ptr<ISong> rhs){
                        
                        return lhs->floatForKey(key) < rhs->floatForKey(key);
                    };
                } else {
                    comp = [=](const shared_ptr<ISong> lhs, const shared_ptr<ISong> rhs){
                        
                        return lhs->floatForKey(key) > rhs->floatForKey(key);
                    };
                }
            } else if (key == "disc") {
                auto ascComp = [=](const shared_ptr<ISong> lhs, const shared_ptr<ISong> rhs){
                        
                    auto l = lhs->uIntForKey(key);
                    auto r = rhs->uIntForKey(key);
                    if (l == 0) {
                        l = 1;
                    }
                    if (r == 0) {
                        r = 1;
                    }
                    return l < r;
                };

                if (asc) {
                    comp = ascComp;
                } else {
                    comp = [=](const shared_ptr<ISong> lhs, const shared_ptr<ISong> rhs){
                        return ascComp(rhs, lhs);
                    };
                }
            } else {
                
                if (asc) {
                    // ascending
                    comp = [key](const shared_ptr<ISong> &lhs, const shared_ptr<ISong> &rhs){
                        
                        //Logger::stream() << "comparekeyasc: " << key << "\n";
                        /*if (key == "title") {
                            if ((std::string)lhs->stringForKey("artist") == "11 II In Taberna" &&
                                (std::string)rhs->stringForKey("artist") == "12 II In Taberna") {

                                std::cout << "hmmm\n";
                            }
                            //Logger::stream() << "'" << (std::string)lhs->stringForKey(key) << "'' vs '" << (std::string)rhs->stringForKey(key) << "'' firstlower: " << MusicSort::lowerThan(lhs->stringForKey(key), rhs->stringForKey(key)) << "\n";
                        }*/
                        return MusicSortCompare::lowerThan(lhs->stringForKey(key), rhs->stringForKey(key));
                    };
                } else {
                    comp = [key](const shared_ptr<ISong> &lhs, const shared_ptr<ISong> &rhs){
                    
                        //Logger::stream() << "comparekeydesc: " << key << "\n";

                        return MusicSortCompare::greaterThan(lhs->stringForKey(key), rhs->stringForKey(key));
                    };
                }
            }
            
            comps.push_back(comp);
        }
        
        return [comps](const SongEntry &lhs, const SongEntry &rhs) {
            
            assert(lhs);
            assert(rhs);
            
            for (auto itComp = comps.begin() ; itComp != comps.end() ; ++itComp) {
                auto comp = *itComp;
                if (comp(lhs.song(),rhs.song())) {
                    return true;
                } else if (comp(rhs.song(),lhs.song())) {
                    return false;
                }
            }
            return false;
        };
    }
}
