#include <string.h>
#include <algorithm>
#include "SongData.h"
#include "json.h"
#include "sfl/Prelude.h"

namespace Gear
{
	using std::make_tuple;
	using std::make_pair;
	using namespace sfl;

#define method SongData::

	vector<tuple<string,size_t,int>> method init_lookup()
	{
        vector<tuple<string,size_t,int>> lookup;
		lookup.push_back(make_tuple("artist",offsetof(SongData,artist),sizeof(artist)));
		lookup.push_back(make_tuple("album",offsetof(SongData,album),sizeof(album)));
		lookup.push_back(make_tuple("title",offsetof(SongData,title),sizeof(title)));
		lookup.push_back(make_tuple("albumArtist",offsetof(SongData,albumArtist),sizeof(albumArtist)));
		lookup.push_back(make_tuple("source",offsetof(SongData,source),sizeof(source)));
		lookup.push_back(make_tuple("playlist",offsetof(SongData,playlist),sizeof(playlist)));
		lookup.push_back(make_tuple("entryId",offsetof(SongData,entryId),sizeof(entryId)));
		lookup.push_back(make_tuple("id",offsetof(SongData,id),sizeof(id)));
		lookup.push_back(make_tuple("genre",offsetof(SongData,genre),sizeof(genre)));
		lookup.push_back(make_tuple("albumArtUrl",offsetof(SongData,albumArtUrl),sizeof(albumArtUrl)));
		lookup.push_back(make_tuple("albumArtUrlHigh",offsetof(SongData,albumArtUrlHigh),sizeof(albumArtUrlHigh)));
		lookup.push_back(make_tuple("albumId",offsetof(SongData,albumId),sizeof(albumId)));
		lookup.push_back(make_tuple("artistId",offsetof(SongData,artistId),sizeof(artistId)));
        lookup.push_back(make_tuple("matchedKey",offsetof(SongData,matchedKey),sizeof(matchedKey)));
		lookup.push_back(make_tuple("offlinePath",offsetof(SongData,offlinePath),sizeof(offlinePath)));
		
		lookup.push_back(make_tuple("disc",offsetof(SongData,disc),Int8));
		lookup.push_back(make_tuple("track",offsetof(SongData,track),Int8));
        lookup.push_back(make_tuple("rating",offsetof(SongData,rating),Int8));
		lookup.push_back(make_tuple("dynamic",offsetof(SongData,dynamic),Int8));

		lookup.push_back(make_tuple("replica",offsetof(SongData,replica),Int16));
		lookup.push_back(make_tuple("playCount",offsetof(SongData,playCount),Int16));
		lookup.push_back(make_tuple("year",offsetof(SongData,year),Int16));

		lookup.push_back(make_tuple("creationDate",offsetof(SongData,creationDate),Int64));
		lookup.push_back(make_tuple("durationMillis",offsetof(SongData,durationMillis),Int64));
		lookup.push_back(make_tuple("lastPlayed",offsetof(SongData,lastPlayed),Int64));

		lookup.push_back(make_tuple("position",offsetof(SongData,position),Float));
		lookup.push_back(make_tuple("offlineRatio",offsetof(SongData,offlineRatio),sizeof(Float)));
		
		std::sort(lookup.begin(),lookup.end());
		return lookup;
    }
    
    vector<tuple<string,size_t,int>> SongData::_lookup = init_lookup();

	pair<int,int> method lookupOffset(const std::string &key)
	{
		auto it = lower_bound(_lookup.begin(),_lookup.end(), make_tuple(key,0,0), 
			[](const tuple<string,size_t,int> &lhs, const tuple<string,size_t,int> &rhs){
				return std::get<0>(lhs) < std::get<0>(rhs);
			});
		if (it == _lookup.end() || std::get<0>(*it) != key) {
			return make_pair(-1,Nada);
		}
		return make_pair(std::get<1>(*it),std::get<2>(*it));
	}

	void method ensureKeysPresent(const vector<string> &keys)
	{
		for (auto &key : keys) {
			auto l = lookupOffset(key);
			if (l.first < 0) {
				string a = intercalate(std::string(","),keys);
				string p = intercalate(std::string(","),map([](const tuple<string,int,int> &t){return std::get<0>(t);},_lookup));
				throw std::runtime_error(std::string("missing key in SongData: ") + key + " of all: " + a + " present: " + p + " count: " + std::to_string(_lookup.size()));
			}
		}
	}

	pair<const void *,int> method lookup(const std::string &key) const
	{
		auto offsetAndType = lookupOffset(key);
		if (std::get<0>(offsetAndType) < 0) {
			return make_pair(nullptr,Nada);
		} 
		return make_pair(reinterpret_cast<const void *>(reinterpret_cast<const char *>(this) + std::get<0>(offsetAndType)),std::get<1>(offsetAndType));
	}

	pair<void *,int> method lookup(const std::string &key)
	{
		const SongData *c = this;
		auto l = c->lookup(key);
		return make_pair(const_cast<void *>(l.first), l.second);
	}

	method SongData()
	{
		memset(this, 0, sizeof(*this));
		valid = false;
	}

	method operator bool() const
	{
		return valid;
	}

	method SongData(const Json::Value &json)
	{
		memset(this, 0, sizeof(*this));
		valid = true;

		for (auto &key : json.getMemberNames()) {
			auto sub = json.get(key, Json::nullValue);
			if (sub.isString()) {
				setStringForKey(key, sub.asString());
			} else if (sub.isIntegral() || sub.isBool()) {

				auto v = sub.asInt64();
				setIntForKey(key, v);
			} else if (sub.isDouble()) {
				auto v = sub.asFloat();
				setFloatForKey(key, v);
			}
		}
	}

	Json::Value method toJson() const
	{
		Json::Value ret;
		for (auto &t : _lookup) {
			auto &key = std::get<0>(t);
			const void *addr = reinterpret_cast<const void *>(reinterpret_cast<const char *>(this) + std::get<1>(t));
			switch(std::get<2>(t)) {
				case Int8:
					ret[key] = *reinterpret_cast<const int8_t *>(addr);
					break;
				case Int16:
					ret[key] = *reinterpret_cast<const int16_t *>(addr);
					break;
				case Int64:
					ret[key] = *reinterpret_cast<const int64_t *>(addr);
					break;
				case Float:
					ret[key] = *reinterpret_cast<const float *>(addr);
					break;
				default:
					ret[key] = string(reinterpret_cast<const char *>(addr));;
					break;
			}
		}
		return std::move(ret);
	}

	string method stringForKey(const string &key) const
	{
		auto l = lookup(key);
        const char *s = reinterpret_cast<const char *>(l.first);
		return l.second > 0 
				? string(s)
				: string("");
	}

	void method setStringForKey(const string &key, const string &value)
	{
		auto l = lookup(key);
		if (l.first != nullptr && l.second > 0) {
			strncpy(reinterpret_cast<char *>(l.first), value.c_str(), l.second);
		}
	}

	float method floatForKey(const string &key) const
	{
		auto l = lookup(key);
		const void *addr = l.first;

		switch(l.second) {
			case Int8:
				return *reinterpret_cast<const int8_t *>(addr);
			case Int16:
				return *reinterpret_cast<const int16_t *>(addr);
			case Int64:
				return *reinterpret_cast<const int64_t *>(addr);
			case Float:
				return *reinterpret_cast<const float *>(addr);
			default:
				return 0;
		}
	}

	int64_t method intForKey(const string &key) const
	{
		auto l = lookup(key);
		const void *addr = l.first;

		switch(l.second) {
			case Int8:
				return *reinterpret_cast<const int8_t *>(addr);
			case Int16:
				return *reinterpret_cast<const int16_t *>(addr);
			case Int64:
				return *reinterpret_cast<const int64_t *>(addr);
			case Float:
				return *reinterpret_cast<const float *>(addr);
			default:
				return 0;
		}
	}


	void method setIntForKey(const string &key, int64_t value)
	{
		setNumberForKey(key, value);
	}

	void method setFloatForKey(const string &key, float value)
	{
		setNumberForKey(key, value);
	}
}