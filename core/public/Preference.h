//
//  Preference.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/26/13.
//
//

#ifndef __G_Ear_Player__Preference__
#define __G_Ear_Player__Preference__

#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include "stdplus.h"

namespace Gear
{
    using std::string;
    using std::function;
    using std::vector;

    class core_export Preference final
    {
    public:
        enum class Type
        {
            Text,
            OnOff,
            // setting a button's value to 1 means button is pressed
            Button,
            Special,
            Options
        };
        enum class OnOffState
        {
            Off = 0,
            On = 1,
            OnSpinning = 2,
            Inactive = 3,
            Disabled = 4
        };
        
        Preference();
        Preference(const Type type, const string &title, const string &key);
        Preference(const Type type, const string &title, const function<void(int64_t)> &changeListener);
        Preference(const Type type, const string &title, const function<void(int64_t)> &changeListener,
        												 const function<int64_t()> &valueFetcher, const vector<string> &options = vector<string>());
        const string &title() const;
        int64_t value() const;
        OnOffState valueOnOff() const;
        void setValue(int64_t value);
        Type type() const;

        Preference &setKey(const std::string &key);
        const string &key() const;
        const vector<string> &options() const;
        
    private:
        Type _type;
        string _key;
        string _title;
        vector<string> _options;
        
        function<void(int64_t)> _changeListener;
        function<int64_t()> _valueFetcher;
    };
}

#endif /* defined(__G_Ear_Player__Preference__) */
