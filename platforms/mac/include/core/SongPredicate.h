//
//  SongPredicate.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__SongPredicate__
#define __G_Ear_Player__SongPredicate__

#include "stdplus.h"
#include <vector>
#include <string>
#include "json.h"

namespace ClientDb
{
    class Predicate;
}

namespace Gear
{
    using std::string;
    using std::vector;
    
    class ISong;
    class IPreferences;
    class SongEntry;
    
    class core_export SongPredicate
    {
    public:
        class Contains final
        {
        };
        
        class Matches final
        {
        };

        class GreaterOrEquals final
        {
        };
        
        SongPredicate();
        // choose fingerprint wisely, e.g. it changes sort order save behaviour, see IPlaylist
        SongPredicate(const string &key, unsigned long long value);
        SongPredicate(const string &key, unsigned long long value, const GreaterOrEquals);
        SongPredicate(const string &key, const string &value, const Matches);
        SongPredicate(const string &key, const string &value, const Contains);
        static SongPredicate recentlyAdded(const IPreferences &preferences);
        bool operator==(const SongPredicate &rhs) const;
        bool operator!=(const SongPredicate &rhs) const;

        explicit operator bool() const;
        operator ClientDb::Predicate() const;
        
        //void analyze(const vector<SongEntry> &all);
        //bool test(const ISong &rhs) const;
        const string &fingerPrint() const;
        const string &key() const;
        std::string value() const;
        
    private:
        //SongPredicate(function<function<bool(const ISong &)>(const vector<SongEntry> &all)> analyzer, const string &fingerPrint);
        
        //function<bool(const ISong &)> _f;
        //function<function<bool(const ISong &)>(const vector<SongEntry> &all)> _analyzer;
        
        string _key;
        Json::Value _value;
        
        string _fingerPrint;
        enum class Operator
        {
            Empty,
            Equals,
            Contains,
            GreaterEquals
        };
        Operator _operator;
    };
}

#endif /* defined(__G_Ear_Player__SongPredicate__) */
