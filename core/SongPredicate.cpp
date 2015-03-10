//
//  SongPredicate.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include <sstream>
#include "SongPredicate.h"
#include "ISong.h"
#include "IPreferences.h"
#include "SongEntry.h"
#include "Predicate.h"

namespace Gear
{    
#define method SongPredicate::
    
    static string createFingerPrint(const string part1, const string part2, const string part3)
    {
        std::stringstream s;
        s << part1 << "/" << part2 << "/" << part3;
        return s.str();
    }
    
    method SongPredicate() :
        _fingerPrint(),
        _operator(Operator::Empty)
    {
    }
    
    method SongPredicate(const string &key, unsigned long long value) :
        _key(key),
        _value(value),
        _operator(Operator::Equals)
    {
        std::stringstream str;
        str << value;
        
        _fingerPrint = createFingerPrint("CONDITION/EQUALS", key, str.str());
    }

    method SongPredicate(const string &key, unsigned long long value, const GreaterOrEquals) :
        _key(key),
        _value(value),
        _operator(Operator::GreaterEquals)
    {
        std::stringstream str;
        str << value;
        
        _fingerPrint = createFingerPrint("CONDITION/GE", key, str.str());
    }

    method SongPredicate(const string &key, unsigned long long value, const GreaterThan) :
        _key(key),
        _value(value),
        _operator(Operator::GreaterThan)
    {
        std::stringstream str;
        str << value;
        
        _fingerPrint = createFingerPrint("CONDITION/GT", key, str.str());
    }

    
    /*
    static function<bool(const ISong &rhs)> filterContainsFunction(const string &key, const string &value)
    {
        if (value.empty()) {
            
            return function<bool(const ISong &rhs)>();
        }
     
        vector<string> filterTokens;
        {
            size_t currentPos = 0;
            while (currentPos < value.size()) {
                
                size_t nextSpace = value.find(" ", currentPos);
                if (nextSpace != std::string::npos) {
                    auto substr = value.substr(currentPos, nextSpace-currentPos);
                    if (!substr.empty()) {
                        filterTokens.push_back(substr);
                    }
                    currentPos = nextSpace+1;
                } else {
                    auto substr = value.substr(currentPos);
                    if (!substr.empty()) {
                        filterTokens.push_back(substr);
                    }
                    break;
                }
            }
        }
        
        vector<string> filterTokensLower;
        transform(filterTokens.begin(), filterTokens.end(), back_inserter(filterTokensLower), [&](const string &rhs){
            return ImmutableString::normalizeS(rhs);
        });
        
        vector<string> keys;
        if (key.empty()) {
            string k[] = {"artistNorm", "albumNorm", "titleNorm", "genreNorm"};
            keys = Base::init<vector<string>>(k);
        } else {
            string k[] = {key};
            keys = Base::init<vector<string>>(k);
        }
        
        return [keys, filterTokensLower](const ISong &rhs){
            
            vector<ImmutableString> values;
            values.reserve(keys.size());
            for (auto it= keys.begin() ; it != keys.end() ; ++it) {
            	auto &key = *it;
            	values.push_back(rhs.stringForKey(key));
            }
            
            for (auto it = filterTokensLower.begin() ; it != filterTokensLower.end() ; ++it) {
            	auto &token = *it;

                bool foundToken = false;
                for (auto itValue = values.begin() ; itValue != values.end() ; ++itValue) {
                	auto &value = *itValue;
                	if (value.contains(token.c_str())) {
                        foundToken = true;
                        break;
                    }
                }
                if (!foundToken) {
                    return false;
                }
            }
            return true;
        };
    }
    */
    method SongPredicate(const string &key, const string &value, const Contains) :
        _fingerPrint(createFingerPrint("CONTAINS", key, value)),
        _key(key),
        _value(value),
        _operator(Operator::Contains)
    {
    }
                                                           
    
    method SongPredicate(const string &key, const string &value, const Matches) :
        _fingerPrint(),
        _key(key),
        _value(value),
        _operator(Operator::Equals)
    {
    }
    
    /*method SongPredicate(function<function<bool(const ISong &)>(const vector<SongEntry> &all)> analyzer, const string &fingerPrint) :
        _fingerPrint(fingerPrint)
    {
    }*/
    
    method operator bool() const
    {
        #pragma message("TODO: SongPredicate validity...")
        return !_fingerPrint.empty();
    }
    
    SongPredicate method recentlyAdded(const IPreferences &preferences)
    {
        #pragma message("TODO: recentlyAdded")
        /*bool showLastItems = preferences.boolForKey("ShowLastItems");
        
        if (showLastItems) {
            long itemsCount = preferences.uintForKey("ShowLastItemsCount");
            function<function<bool(const ISong &)>(const vector<SongEntry> &all)> analyzer = [itemsCount](const vector<SongEntry> &all){
                
                if (itemsCount < 1 || all.size() < 1) {
                    function<bool(const ISong &)> testFunction = [](const ISong &rhs){
                        
                        return false;
                    };
                    return testFunction;
                }
                
                unsigned long size = itemsCount;
                if (size > all.size()) {
                    size = all.size();
                }
                vector<SongEntry> mostRecent(size);
                partial_sort_copy(all.begin(), all.end(), mostRecent.begin(), mostRecent.end(), [](const SongEntry &lhs, const SongEntry &rhs){
                    return lhs.song()->uIntForKey("creationDate") > rhs.song()->uIntForKey("creationDate");
                });
                auto borderSong = (*mostRecent.rbegin());
                long long borderNanosecs = borderSong.song()->uIntForKey("creationDate");
                //string borderId = borderSong->uniqueId();
                
                function<bool(const ISong &)> testFunction = [borderNanosecs](const ISong &rhs){
                    
                    auto date = rhs.uIntForKey("creationDate");
                    return (date >= borderNanosecs);
                };
                return testFunction;
            };
            
            return SongPredicate(analyzer, "LASTADDED");
            
        } else {
            long daysCount = preferences.uintForKey("ShowLastDaysCount");
            if (daysCount == 0) {
                // bulletproof even if there is no gui to set this and we've forgotten to set the defaults 
                daysCount = 14;
            }
            
            long long now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            
            long long border = now -daysCount * 24 * 60 * 60;
            long long borderNanosecs = border * 1000000;
            
            function<bool(const ISong &)> f = [borderNanosecs](const ISong &rhs){
                auto date = rhs.uIntForKey("creationDate");
                if (date >= borderNanosecs) {
                    return true;
                } else{
                    return false;
                }
            };
            SongPredicate ret(f, "LASTDAYS");
            return ret;
        }*/
        return SongPredicate();
    }
    
    /*bool method test(const ISong &rhs) const
    {
        if (_f) {
            return _f(rhs);
        } else {
            return true;
        }
    }
    
    void method analyze(const vector<SongEntry> &all)
    {
        if (_analyzer) {
            _f = _analyzer(all);
        }
    }*/
    
    bool method operator==(const SongPredicate &rhs) const
    {
        if (_fingerPrint.empty() || rhs._fingerPrint.empty()) {
            return false;
        }
        return _fingerPrint == rhs._fingerPrint;
    }
    
    bool method operator!=(const SongPredicate &rhs) const
    {
        return !operator==(rhs);
    }

    
    const string & method fingerPrint() const
    {
        return _fingerPrint;
    }
    
    const string & method key() const
    {
        return _key;
    }
    
    std::string method value() const
    {
        return _value.asString();
    }

    method operator ClientDb::Predicate() const
    {
        string key = _key;
        if (key == "artistNorm") {
            key = "artist";
        } else if (key == "albumNorm") {
            key = "album";
        } else if (key == "titleNorm") {
            key = "title";
        }

        switch(_operator) {
            case Operator::Empty:
                return ClientDb::Predicate();
            case Operator::Equals:
                return ClientDb::Predicate(ClientDb::Predicate::Operator::Equals, key, _value);
            case Operator::Contains:
                if (_value.empty()) {
                    return ClientDb::Predicate();
                }
                if (key.empty()) {
                    vector<ClientDb::Predicate> subs;
                    vector<std::string> keys;
                    keys.push_back(std::string("artist"));
                    keys.push_back(std::string("album"));
                    keys.push_back(std::string("title"));
                    keys.push_back(std::string("genre"));
                    keys.push_back(std::string("comment"));
                    for (auto &subkey : keys) {
                        subs.push_back(ClientDb::Predicate(ClientDb::Predicate::Operator::Contains, subkey, _value));
                    }
                    return ClientDb::Predicate(ClientDb::Predicate::Operator::Or, subs);
                } else {
                    return ClientDb::Predicate(ClientDb::Predicate::Operator::Contains, key, _value);
                }
            case Operator::GreaterEquals:
                return ClientDb::Predicate(ClientDb::Predicate::Operator::GreaterEquals, key, _value);
            case Operator::GreaterThan:
                return ClientDb::Predicate(ClientDb::Predicate::Operator::GreaterThan, key, _value);
        }
        throw std::runtime_error("unsupported operator");
    }
}
