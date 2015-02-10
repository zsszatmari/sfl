//
//  Preference.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/26/13.
//
//

#include "Preference.h"
#include "IApp.h"
#include "IPreferences.h"

namespace Gear
{
#define method Preference::
    
    method Preference()
    {
    }
    
    method Preference(const Type type, const string &title, const string &key) :
        _type(type),
        _title(title),
        _changeListener([key](int64_t value){
            IApp::instance()->preferences().setUintForKey(key, value);
        }),
        _valueFetcher([key]{
            return IApp::instance()->preferences().uintForKey(key);
        })
    {
    }
    
    method Preference(const Type type, const string &title, const function<void(int64_t)> &changeListener) :
        _type(type),
        _title(title),
        _changeListener(changeListener),
        _valueFetcher([]{
            return 0;
        })
    {
    }
    
    method Preference(const Type type, const string &title, const function<void(int64_t)> &changeListener,
    				  const function<int64_t()> &valueFetcher, const vector<string> &options) :
            _type(type),
            _title(title),
            _changeListener(changeListener),
            _valueFetcher(valueFetcher),
            _options(options)
	{
	}

    const vector<string> & method options() const
    {
        return _options;
    }

    Preference::Type method type() const
    {
        return _type;
    }
    
    const string & method title() const
    {
        return _title;
    }
    
    int64_t method value() const
    {
    	return _valueFetcher();
    }

    const string & method key() const
    {
        return _key;
    }

    Preference & method setKey(const std::string &key)
    {
        _key = key;
        return *this;
    }
    
    Preference::OnOffState method valueOnOff() const
    {
    	return static_cast<Preference::OnOffState>(_valueFetcher());
    }
    
    void method setValue(int64_t value)
    {
    	_changeListener(value);
    }
}
