//
//  YoutubeProtocol.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/14/13.
//
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "json.h"
#include "stdplus.h"
#include "YouTubeProtocol.h"
#include "UrlEncode.h"
#include "YouTubeDecode.h"
#include "IJavascriptEngine.h"

namespace Gear
{
#define method YouTubeProtocol::
    
    using std::map;
    
    map<string, string> parse(const string &params, const string &separator = "&")
    {
        map<string, string> parsed;
        
        string key;
        string value;
        bool parsingValue = false;
        
        auto end = [&]{
            if (!key.empty()) {
                parsed.insert(std::make_pair(key, value));
                parsingValue = false;
                //std::cout << key << " = " << value << std::endl;
                
                key = "";
                value = "";
            }
        };
        
        for (int i = 0 ; i < params.length() ; ++i) {

        	if (i + separator.length() <= params.length() && params.compare(i, separator.length(), separator) == 0) {
        		end();
        		i += (separator.length() -1);
        	} else {

        		char c = params[i];
            
				if (c == '=') {
					parsingValue = true;
				} else {
					if (parsingValue) {
						value.push_back(c);
					} else {
						key.push_back(c);
					}
				}
        	}
        }
        end();
        return parsed;
    }
    
    static string videoUrlFromStreamMapWithQuality(const string &streamMap, const string &separator, const shared_ptr<IJavascriptEngine> &engine, const string &requestedQuality)
    {
        std::stringstream ss(streamMap);
        string current;
        //vector<string> mapParts;
        
        //map<string,string> typeToUrl;
        
        while (std::getline(ss, current, ',')) {
            
            string mapPartStr = current;
            auto part = parse(mapPartStr, separator);
            auto itType = part.find("type");
            auto itSig = part.find("sig");
            auto itEncrypted = part.end();
            if (engine) {
                itEncrypted = part.find("s");
            }
            auto itUrl = part.find("url");
            auto itQuality = part.find("quality");
            
            if (itType != part.end() && itUrl != part.end() && (itSig != part.end() || itEncrypted != part.end() || (itUrl->second.find("signature%3D") != string::npos))) {
                string type = UrlEncode::decode(itType->second);
                string mp4Prefix("video/mp4");
                if (type.compare(0, mp4Prefix.length(), mp4Prefix) != 0) {
                    continue;
                }
                string quality;
                if (itQuality != part.end()) {
                	quality = itQuality->second;
                }
                if (!requestedQuality.empty() && requestedQuality != quality) {
                	continue;
                }
                
                string signature;
                if (itSig != part.end()) {
                    signature = UrlEncode::decode(itSig->second);
                } else if (itEncrypted != part.end()) {
                    signature = engine->execute("decode('" + itEncrypted->second + "')");
                }
                string url = UrlEncode::decode(itUrl->second) + (signature.empty() ? "" : ("&signature=" + signature));
                //std::cout << type << " : " << url << std::endl;

                return url;
                //typeToUrl.insert(std::make_pair(type, url));
            }
            //mapParts.push_back();
        }
        return "";
    }

    static string videoUrlFromStreamMap(const string &streamMap, const string &separator, const shared_ptr<IJavascriptEngine> &engine)
    {
    	auto result = videoUrlFromStreamMapWithQuality(streamMap, separator, engine, "medium");
    	if (!result.empty()) {
    		return result;
    	}
    	result = videoUrlFromStreamMapWithQuality(streamMap, separator, engine, "");
    	return result;
    }

    string method videoUrl(const string &videoPage)
    {
    	auto parsed = parse(videoPage);

		auto it = parsed.find("url_encoded_fmt_stream_map");
		if (it == parsed.end()) {
			return "";
		}
		return videoUrlFromStreamMap(UrlEncode::decode(it->second), "&", shared_ptr<IJavascriptEngine>());
    }
    
    static string playerUrl(const string &videoInfo)
    {
        size_t pos = videoInfo.find("\"assets\":");
        if (pos == string::npos) {
            return "";
        }
        int braces = 0;
        int beginPos = 0;
        do {
            pos = videoInfo.find_first_of("{}", pos);
            if (videoInfo.at(pos) == '{') {
                if (beginPos == 0) {
                    beginPos = pos;
                }
                ++braces;
            } else if (videoInfo.at(pos) == '}') {
                --braces;
            }
            ++pos;
        } while (braces > 0);
        ++pos;
        auto assetsStr = videoInfo.substr(beginPos, pos-beginPos);
        Json::Reader reader;
        Json::Value assets;
        if (!reader.parse(assetsStr, assets)) {
            return "";
        }
        string playerUrl = assets.get("js","").asString();
        if (playerUrl.compare(0,2,"//") == 0) {
            playerUrl.insert(0, "http:");
        }
        return playerUrl;
    }

    string method videoUrlWeb(const string &videoInfo)
    {
#ifdef DEBUG
        {
            std::ofstream fs("/Users/zsszatmari/javascriptdebug0-videoinfo.txt");
            fs << videoInfo;
        }
#endif
        auto player = playerUrl(videoInfo);
        static YouTubeDecode decoder;
        auto javascriptEngine = decoder.engine(player);
        
        size_t f;
    	string prefix = "\"url_encoded_fmt_stream_map\":\"";
    	f = videoInfo.find(prefix);
        if (f == string::npos) {
            prefix = "\"url_encoded_fmt_stream_map\": \"";
            f = videoInfo.find(prefix);    
        }
    	if (f == string::npos) {
    		return "";
    	}
    	f += prefix.size();
    	auto end = videoInfo.find("\"", f);
    	if (end == string::npos) {
    		return "";
    	}

    	string streamMap = videoInfo.substr(f, end-f);

    	return videoUrlFromStreamMap(streamMap, "\\u0026", javascriptEngine);
    }
}
