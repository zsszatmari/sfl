//
//  QueueSongArray.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 28/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <sstream>
#include "IApp.h"
#include "Db.h"
#include "IPlayer.h"
#include "QueueSongArray.h"
#include "MainExecutor.h"
#include "IoService.h"

namespace Gear
{
#define method QueueSongArray::

    shared_ptr<QueueSongArray> method create(const shared_ptr<IPlayer> &player)
    {
        shared_ptr<QueueSongArray> ret(new QueueSongArray(player));
        ret->init();
        return ret;
    }
    
    shared_ptr<QueueSongArray> method instance()
    {
        return MEMORY_NS::static_pointer_cast<QueueSongArray>(IApp::instance()->queueSongArray());
    }

    method QueueSongArray(const shared_ptr<IPlayer> player) :
        SortedSongArray(IApp::instance()->db(), ClientDb::Predicate(ClientDb::Predicate::Operator::Equals, "playlist", "queue")),
        _player(player)
    {
    }

    void method recalculate()
    {
        // must be called when potential there is a change, i.e. change search text
        _all.reset();

        recalculate(_player->songEntryConnector().value());
    }

    void method init()
    {
        static int count = 0;

        auto c = [this]{
            recalculate();
        };

        _repeatConnection = _player->repeat().connector().connect([c](IPlayer::Repeat){
            if (++count > 2) {
                c();
            }
        });
        // no shared_from_this, assume that this object lives forever
        //auto self = shared_from_this();
         _shuffleConnection = _player->shuffle().connector().connect([c,this](bool shuffle){
            if (++count > 2) {
                if (shuffle) {
                    shared_ptr<ISongArray> array = _arrayForJumping;
                    auto entry = _player->songEntryConnector().value();
                    if (array && entry) {
                        // if the user repeatedly clicks on shuffle, reshuffle songs
                        array->setNoRepeatBase(entry);
                    }
                }
                c();
            }
        });
    }
    
    void method recalculate(const shared_ptr<ISongArray> &array, const SongEntry &entry)
    {
#pragma message("PlayerBase ezt hivja meg main threadbol, ami megakaszt")
        
        if (array.get() == this) {
            recalculate(entry);
            return;
        }

        if (((shared_ptr<ISongArray>)_arrayForJumping) != array) {
            _all.reset();
        }
        _outOfOrder.setS([&](shared_ptr<vector<SongEntry>> &outOfOrder){
            outOfOrder->erase(remove(outOfOrder->begin(), outOfOrder->end(), entry), outOfOrder->end());
            return outOfOrder;
        });
        _arrayForJumping = array;
        if (array) {
            _updateConnection = array->songs([this](const SongView &songView, SongView::Event event, size_t offset, size_t size){
                //std::cout << "queue recalculate should begin because of change\n";
                // note: if there is no recalculation, most likely array is set to null!
                recalculate();
            });
        } else {
            //abort();
            _updateConnection = SongView();
        }
        recalculate(entry);
    }
    
    void method enqueue(const vector<SongEntry> &entries, bool asNext)
    {
        size_t place = 0;
        _outOfOrder.setS([&](shared_ptr<vector<SongEntry>> &all){
            auto it = asNext ? all->begin() : all->end();
            place = it - all->begin();
            all->insert(it, entries.begin(), entries.end());
            return all;
        });
        auto playing = _player->songEntryConnector().value();
        if (!playing) {
            return;
        }
        _all.setS([&](shared_ptr<vector<SongEntry>> &all){
            auto it = find(all->begin(), all->end(), playing);
            if (it == all->end()) {
                return all;
            }
            ++it;
            if ((it - all->begin()) + place > all->size()) {
                it = all->end();
            } else {
                it += place;
            }
            
            all->insert(it, entries.begin(), entries.end());
            return postprocess(all);
        });
        
        setAllSongs(_all);
    }
    
    void method removeFromQueue(const vector<SongEntry> &entries)
    {
        // important: it's not just the out of orders
        _all.setS([&](shared_ptr<vector<SongEntry>> &all){
            for (auto &entry : entries) {
                all->erase(remove(all->begin(), all->end(), entry), all->end());
            }
            return postprocess(all);
        });
        _outOfOrder.setS([&](shared_ptr<vector<SongEntry>> &all){
            for (auto &entry : entries) {
                all->erase(remove(all->begin(), all->end(), entry), all->end());
            }
            return all;
        });
        
        setAllSongs(_all);
    }
    
    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        _all.setS([&](shared_ptr<vector<SongEntry>> &all){
            all->erase(remove_if(all->begin(), all->end(), [&](const SongEntry &entry){
                return find(entries.begin(), entries.end(), entry) != entries.end();
            }), all->end());
            
            bool foundIt = false;
            auto it = all->end();
            if (after) {
                it = find(all->begin(), all->end(), after);
                if (it != all->end()) {
                    foundIt = true;
                    ++it;
                }
            }
            if (!foundIt) {
                it = find(all->begin(), all->end(), before);
                if (it == all->end()) {
                    return all;
                }
            }
            
            all->insert(it, entries.begin(), entries.end());
            
            return postprocess(all);
        });
        
        /*_outOfOrder->setS([&](shared_ptr<vector<SongEntry>> &outOfOrder){
        });*/
        
        setAllSongs(_all);
        return true;
    }
    
    SongEntry method whatToPlayNextAfter(const SongEntry &songPlayed, bool forward)
    {
        recalculate(songPlayed);
        
        auto all = _all.get();
        auto it = find(all->begin(), all->end(), songPlayed);
        if (it == all->end()) {
            return nextInOrder(songPlayed, forward).unclone();
        }
//        std::cout << "current song: " << (it - all->begin()) << (std::string)it->song()->title() << std::endl;
        if (forward) {
            ++it;
            shared_ptr<ISongArray> arrayForJumping = _arrayForJumping;
        
            if (it == all->end()) {
                if (arrayForJumping) {
                    arrayForJumping->songsLeft(0);
                }
                return nextInOrder(songPlayed, forward).unclone();
            } else {
//                std::cout << "next song: " << (it - all->begin()) << (std::string)it->song()->title() << std::endl;
                auto ret = it->unclone();
                if (arrayForJumping) {
                    arrayForJumping->songsLeft((all->end() - it) -1);
                }
                return ret;
            }
        } else {
            if (it == all->begin()) {
                return nextInOrder(songPlayed, forward).unclone();
            } else {
                --it;
                return it->unclone();
            }
        }
    }
    
    void method recalculate(const SongEntry &centered)
    {
        // if we are producing very bad results it's because there are multiple items with same unique id

        if (!centered) {
            return;
        }
        //std::cout << "queue recalculate.. " << centered.song()->uniqueId() << "\n";
    
        auto self = shared_from_this();
        //_queueCalculator.addTask(
        Io::get().dispatch([self,this,centered]{

            shared_ptr<ISongArray> array = _arrayForJumping;
            decltype(array->inspectionContext()) context;
            if (array) {
                context = array->inspectionContext();
            }
            
            shared_ptr<const vector<SongEntry>> outOfOrder = _outOfOrder;
            
            _all.setS([&](shared_ptr<vector<SongEntry>> &all){
                size_t centerPos;
                SongEntry current = centered;
                /*std::cout << "queue 0th phase: " << all->size() << std::endl;
                for (auto song : *all) {
                    std::cout << "queue song: " << song.song()->uniqueId() << std::endl;
                }*/

                {
                    auto itCenter = find(all->begin(), all->end(), centered);
                    if (itCenter == all->end()) {
                        all->clear();
                        all->push_back(centered);
                        itCenter = all->begin();
                    }
                    
                    centerPos = itCenter - all->begin();
                    
                    for (auto itCurrent = all->begin() ; itCurrent != itCenter ; ++itCurrent) {
                        bool inOrder = find(outOfOrder->begin(), outOfOrder->end(), *itCurrent) == outOfOrder->end();
                        if (inOrder) {
                            current = *itCurrent;
                            break;
                        }
                    }
                }

                //std::cout << "queue 1st phase: " << all->size() << std::endl;
                //std::cout << "queue based on: " << current.entryId() << std::endl;

                static const int kBefore = 5;
                static const int kAfter = 20;
                
                while(centerPos < kBefore) {
                    current = nextInOrder(current, false).cloneWith(current.replica() -1);
                    //std::cout << "queue before result: " << current.entryId() << std::endl;

                    if (!current) {
                        break;
                    }
                    all->insert(all->begin(), current);
                    ++centerPos;
                }
                //std::cout << "queue 2nd phase: " << all->size() << std::endl;

                while(centerPos > kAfter) {
                    all->erase(all->begin());
                    --centerPos;
                }
                //std::cout << "queue 3rd phase: " << all->size() << std::endl;

                
                for (auto &entry : *outOfOrder) {
                    auto it = find(all->begin(), all->end(), entry);
                    if (it == all->end()) {
                        all->insert(all->begin() + centerPos+1, entry);
                        ++centerPos;
                    }
                }
                //std::cout << "queue 4th phase: " << all->size() << std::endl;
                
                current = centered;
                {
                    auto itCenter = find(all->begin(), all->end(), centered);
                    if (itCenter == all->end()) {
                        // impossible
                        return all;
                    }
                    for (auto itCurrent = all->end()-1 ; itCurrent != itCenter ; --itCurrent) {
                        bool inOrder = find(outOfOrder->begin(), outOfOrder->end(), *itCurrent) == outOfOrder->end();
                        if (inOrder) {
                            current = *itCurrent;
                            break;
                        }
                    }
                }
                //std::cout << "queue 5th phase: " << all->size() << std::endl;
                
                while ((all->size() - centerPos) < kAfter) {
                    //std::cout << "queue after based on: " << current.entryId() << std::endl;
                    current = nextInOrder(current, true).cloneWith(current.replica() +1);
                    //std::cout << "queue after result: " << current.entryId() << std::endl;

                    if (!current) {
                        break;
                    }
                    all->insert(all->end(), current);
                }

                /*for (auto &entry : *all) {
                    std::cout << "queue newcontent: " << entry.entryId() << std::endl;
                }*/

                //std::cout << "queue recalculate: " << all->size() << "\n";
                return postprocess(all);
            });
        //std::cout << "queue recalculate end..\n";
            setAllSongs(_all);
        });
        
    }

    SongEntry method nextInOrder(const SongEntry &songPlayed, bool forward)
    {
        if (!songPlayed) {
            return SongEntry();
        }
        
#ifdef DEBUG_WHATTOPLAY
        //NSLog(@"DEBUG what to play next?. repeat state: %d currentsong: %@,%@", self.repeatButton.intValue, songPlayed.songId, songPlayed.title);
#endif
        auto shuffle = _player->shuffle().get();
        auto repeat = _player->repeat().get();
        
        if (repeat == IPlayer::Repeat::OneSong) {
            return songPlayed.clone(forward ? +1 : -1);
        }
        
        SongEntry nextSong;
        shared_ptr<ISongArray> arrayForJumping = _arrayForJumping;
        
        if (!shuffle) {
            
            // arrayForJumping contains objects, but can contain deleted items as well, so only id's should be used.
            //assert(_arrayForJumping);
            if (!arrayForJumping) {
                return SongEntry();
            }

            #pragma message("TODO: queue calculation missing")
            
            auto size = arrayForJumping->sizeSync();
            if (size == 0) {
                return SongEntry();
            }
            
            // indexOf must be called in the Io thread
            assert(Io::isCurrent());

            auto index = arrayForJumping->indexOf(songPlayed.unclone());
            /*if (songPlayed.entryId() == "c" && index == 0) {
                abort();
            }*/

            //std::cout << "queue index for " << songPlayed.song()->uniqueId() << "," << songPlayed.entryId() << ": " << index << "/" << size << std::endl;
            if (index == ISongArray::NotFound || index >= size) {
                return SongEntry();
            }
            // this is wrong here
            //arrayForJumping->songsLeft(collection->size() - index -1);
            
            //NSAssert4([[collection objectAtIndex:index] isEqual:songPlayed],@"index mismatch1: %ld search for %@ found instead: %@ total count: %ld",index,songPlayed, [collection objectAtIndex:index], [collection count]);
            
            long nextIndex;
            nextIndex = index + (forward ? +1 : -1);
            if (nextIndex >= size) {
                if (repeat == IPlayer::Repeat::On) {
                    nextIndex = 0;
                } else {
                    return SongEntry();
                }
            }
            if (nextIndex < 0) {
                if (repeat == IPlayer::Repeat::On) {
                    nextIndex = size-1;
                } else {
                    return SongEntry();
                }
            }
            
            SongEntry songCandidate = arrayForJumping->atSync(nextIndex);
            if (songCandidate) {
                nextSong = songCandidate;
                //std::cout << "next item: " << songCandidate.song()->uniqueId() << "   index: " << nextIndex << std::endl;
            }
        } else {
            
            if (!arrayForJumping) {
                return SongEntry();
            }
            return arrayForJumping->nextShuffledSongRelativeTo(songPlayed.unclone(), forward, repeat == IPlayer::Repeat::On);
        }
        return nextSong;
    }
    
    const bool method orderedArray() const
    {
        return true;
    }

    void method setNoRepeatBase(const SongEntry &song)
    {
        // following is wrong, because user does not expect recalculation
        // just ignore setnorepeatbase and we'll be fine
        /*shared_ptr<ISongArray> jumping = _arrayForJumping;
        if (jumping) {
            jumping->setNoRepeatBase(song);
        }*/
    }

    static bool findIn(const vector<SongEntry> &songs, const SongEntry &toFind)
    {
        //return find(songs.begin(),songs.end(), playingEntry) != songs.end();
        for (auto &song : songs) {
            if (toFind.song()->uniqueId() == toFind.song()->uniqueId()) {
                return true;
            }
        }
        return false;
    }

    void method willRemoveSongs(const vector<SongEntry> &songs)
    {
        auto player = IApp::instance()->player();
        for (int i = 0 ; i < songs.size() ; ++i) {
            SongEntry playingEntry = player->songEntryConnector().value();
            if (playingEntry && findIn(songs, playingEntry)) {
                //std::cout << "queue deleted currently playing\n";
                player->next();
            } else {
                break;
            }
        }
    }

    shared_ptr<vector<SongEntry>> method postprocess(const shared_ptr<vector<SongEntry>> &v)
    {
        // add entry ids to the mix otherwise there will be only one entry after all
        int order = 0;
        for (auto &s : *v) {

            std::stringstream ss;
            ss << order;
            // don't change the entry id either, find another way to distinguish
            //s.song()->setStringForKey("entryId", ss.str());
            s.song()->setIntForKey("position", order);
            s.song()->setStringForKey("playlist", "queue");
            // must not change the source: it would be impossible to find the originating session
            //s.song()->setStringForKey("source","queue");
            ++order;
        }
        return v;
    }
}