//
//  PredicateSongArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "PredicateSongArray.h"
#include "IApp.h"
#include "IPlayer.h"
#include "MainExecutor.h"
#include "Db.h"

namespace Gear
{
#define method PredicateSongArray::
    
    shared_ptr<PredicateSongArray> method create(const shared_ptr<SortedSongArray> &base, const SongPredicate &predicate)
    {
        shared_ptr<PredicateSongArray> ret(new PredicateSongArray(base, predicate));
        ret->initUpdate(base);
        return ret;
    }
    
    method PredicateSongArray(const shared_ptr<SortedSongArray> &base, const SongPredicate &predicate) :
        SortedSongArray(IApp::instance()->db(), ClientDb::Predicate::compositeAnd(base->inherentPredicate(),predicate)),
        _base(base)
    {
        setSecondaryFilterPredicate(predicate);
    }
    
    void method initUpdate(const shared_ptr<SortedSongArray> &base)
    {
    	// would cause infinite loop
        //shared_ptr<SortedSongArray> pThis = shared_from_this();
        //base->propertyChangeSignal().connect(&PredicateSongArray::doAlertListener, pThis);

    	shared_ptr<PredicateSongArray> pThis = shared_from_this();
    	//IApp::instance()->player()->ratedConnector().connect(&PredicateSongArray::doAlertListener, pThis);
        
    	weak_ptr<PredicateSongArray> wThis = pThis;
        _updateConnection = base->updatedEvent().connect([wThis]{
            auto p = wThis.lock();
    		if (p) {
    			//p->alertListener();
    			//p->setFilterPredicate(p->filterPredicate());
                
    			p->resetCache();
            }
        });

    	_ratedConnection = IApp::instance()->player()->ratedConnector().connect([wThis]{
    		auto p = wThis.lock();
    		if (p) {

                //#warning hooooooo
                //p->resetCache();
    			//p->alertListener();
    			//p->setFilterPredicate(p->filterPredicate());

                //if (IApp::instance()->phoneInterface()) {
                    // on desktop if we do this instantly,  this would mean impossible downrate from unrated auto playlist if there are few songs
                p->_rateTimer = Timer([wThis](Timer *t){

                    MainExecutor::instance().addTask([wThis]{
                        auto self = wThis.lock();
                        if (self) {
                            self->resetCache();
                        }
                    });
                }, 5.0f, false, 0.5f);
                //}
    		}
    	});
        resetCache();
    }
    
    void method resetCache()
    {
        //auto songs = _base->rawSongs();
    	//setRawSongs(songs);
    }
    
    vector<shared_ptr<ISongIntent>> method songIntents(const vector<SongEntry> &songs, const string &fieldHint) const
	{
    	auto s = _base;
    	if (s) {
    		return s->songIntents(songs, fieldHint);
    	}
    	return vector<shared_ptr<ISongIntent>>();
	}

	vector<pair<string, vector<shared_ptr<ISongIntent>>>> method songIntentSubMenus(const vector<SongEntry> &songs) const
	{
		auto s = _base;
		if (s) {
			return s->songIntentSubMenus(songs);
		}
		return vector<pair<string, vector<shared_ptr<ISongIntent>>>>();
	}

}
