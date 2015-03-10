#ifndef SONGDATA_H
#define SONGDATA_H

#include <vector>
#include <string>
#include <tuple>
#include "json-forwards.h"

namespace Gear
{
	using std::vector;
	using std::tuple;
	using std::string;
	using std::pair;

	class SongData final
	{
	public:
		SongData();
		SongData(const Json::Value &json);
		Json::Value toJson() const;
		operator bool() const;
		string stringForKey(const string &key) const;
		float floatForKey(const string &key) const;
		int64_t intForKey(const string &key) const;
		static void ensureKeysPresent(const vector<string> &keys);
		void setStringForKey(const string &key, const string &value);
		void setFloatForKey(const string &key, float value);
		void setIntForKey(const string &key, int64_t value);

		template<typename T>
		void setNumberForKey(const string &key, T value);

	private:
		char artist[80];
		char album[80];
		char albumArtist[80];
		char title[80];
		char source[10];
		char playlist[80];
		char entryId[80];
		char genre[40];
		char id[80];
		char albumArtUrl[80];
		char albumArtUrlHigh[80];
		char albumId[80];
		char artistId[80];
		char matchedKey[80];
		char offlinePath[80];

		int64_t durationMillis;
		int64_t lastPlayed;
		int64_t creationDate;

		float position;
		float offlineRatio;

		int16_t year;
		int16_t playCount;
		int16_t replica;
		int8_t rating; 
		int8_t dynamic;
		int8_t disc;
		int8_t track;
		bool valid;

		static vector<tuple<string,size_t,int>> init_lookup();
		static vector<tuple<string,size_t,int>> _lookup;

		static pair<int,int> lookupOffset(const std::string &key);


		pair<const void *,int> lookup(const std::string &key) const;
		pair<void *,int> lookup(const std::string &key);

		enum Type 
		{
			// positive values are all string field lengths
			Int8 = -1,
			Int16 = -2,
			Int64 = -4,
			Float = -5,
			Nada = -6
		};
	};

#define method SongData::

	template<typename T>
	void method setNumberForKey(const string &key, T v)
	{
		auto l = lookup(key);
		if (l.first != nullptr) {
			switch(l.second) {
				case Int8: {
					int8_t &value = *reinterpret_cast<int8_t *>(l.first);
					value = v;
					break;
				}
				case Int16: {
					int16_t &value = *reinterpret_cast<int16_t *>(l.first);
					value = v;
					break;
				}
				case Int64: {
					int64_t &value = *reinterpret_cast<int64_t *>(l.first);
					value = v;
					break;
				}
				case Float: {
					float &value = *reinterpret_cast<float *>(l.first);
					value = v;
					break;
				}
			}
		}
	}

#undef method
}

#endif