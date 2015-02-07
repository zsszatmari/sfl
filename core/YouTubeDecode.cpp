//
//  YouTubeSignature.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 16/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <fstream>
#include "YouTubeDecode.h"
#include "SerialExecutor.h"
#include "HttpDownloader.h"
#include "IApp.h"
#include "IJavascriptEngine.h"

namespace Gear
{
#define method YouTubeDecode::
    
    static string entryFunction(const string &js)
    {
        const string sig("signature=");
        size_t sigPos = js.find(sig);
        if (sigPos != string::npos) {
            sigPos += sig.length();
        } else {
            const string alternSig("set(\"signature\",");
            sigPos = js.find(alternSig);
            if (sigPos != string::npos) {
                sigPos += alternSig.length();
            } else {
                return "";
            }
        }
        size_t bracePos = js.find("(", sigPos);
        if (bracePos == string::npos) {
            return "";
        }
        return js.substr(sigPos, bracePos - sigPos);
    }
    
    static void extractFunction(const string &js, const string &fnname, vector<string> &functions, string &result)
    {
        string definition = "function " + fnname + "(";
        size_t defPos = js.find(definition);
        bool isStruct = false;
        size_t pos;
        if (defPos == string::npos) {
            definition = "var " + fnname + "={";
            defPos = js.find(definition);
            if (defPos == string::npos) {
                return;
            }
            isStruct = true;
            pos = defPos + definition.length() -1;
            // let it find the first, opening curly brace
        } else {
            pos = defPos;
        }
        int braceCount = 0;
        
        vector<string> needFunctions;
        do {
            pos = js.find_first_of("{}=;", pos);
            if (pos == string::npos) {
                return;
            }
            if (js.at(pos) == '{') {
                ++braceCount;
            } else if (js.at(pos) == '}') {
                --braceCount;
            } else if (!isStruct) {
                if (js.at(pos) == '=') {
                    
                    ++pos;
                    size_t subpos = js.find_first_of("(", pos);
                    if (subpos == string::npos) {
                        return;
                    }
                    string candidate = js.substr(pos, subpos - pos);
                    if (all_of(candidate.begin(),candidate.end(),isalnum)) {
                        if (find(functions.begin(), functions.end(), candidate) == functions.end() &&
                            find(needFunctions.begin(), needFunctions.end(), candidate) == needFunctions.end()) {
                            needFunctions.push_back(candidate);
                        }
                    }
                } else if (js.at(pos) == ';') {
                    // var ar={g7:function(a,b){a.splice(0,b)},ah:function(a,b){var c=a[0];a[0]=a[b%a.length];a[b]=c},CH:function(a){a.reverse()}};
                    if (js.length() > pos+3 && js.at(pos+3) == '.') {
                        string candidate = js.substr(pos+1,2);
                        if (all_of(candidate.begin(),candidate.end(),isalnum)) {
                            if (find(functions.begin(), functions.end(), candidate) == functions.end() &&
                                find(needFunctions.begin(), needFunctions.end(), candidate) == needFunctions.end()) {
                                needFunctions.push_back(candidate);
                            }
                        }
                    }
                }
            }
            ++pos;
        } while (braceCount > 0);
        
        functions.push_back(fnname);
        result.append(js, defPos, pos-defPos);
        result.append(";");
#ifdef DEBUG
        //std::cout << "got fn: " << js.substr(defPos, pos-defPos) << std::endl;
#endif
        
        for (auto &need : needFunctions) {
            
            if (find(functions.begin(), functions.end(), need) == functions.end()) {
                extractFunction(js, need, functions, result);
            }
        }
    }

// this won't work with tinyjs, and not even with duktape
/*
    static string extractFunction(const string &aJs, const string &fnname)
    {
        const string declarations("var window = {history:{},location:{}};var document = {};var navigator = {};");
        const string unneededPrefix("(function(){");
        const string unneededPostfix("})();");

        string js = aJs;
        while (js.length() > 0 && (js[js.length()-1] == '\n' || js[js.length()-1] == '\r')) {
            js.resize(js.length()-1);
        }
        if (js.compare(0,unneededPrefix.length(),unneededPrefix) == 0 && 
            js.compare(js.length()-unneededPostfix.length(), unneededPostfix.length(), unneededPostfix) == 0) {

            return declarations + js.substr(unneededPrefix.length(), js.length() -unneededPrefix.length() - unneededPostfix.length());
        }
        return "";
    }
*/
    static string extractFunction(const string &js, const string &fnname)
    {
        vector<string> functions;
        string result;
        extractFunction(js, fnname, functions, result);
        return result;
    }
    
    shared_ptr<IJavascriptEngine> method engine(const std::string &playerUrl)
    {
        //std::cout << "player url: " << playerUrl << std::endl;
        
        auto currentEngine = _currentEngine;
        if (currentEngine->first == playerUrl) {
            auto got = currentEngine->second;
            if (got) {
                return got;
            }
            // else retry
        }
        
        // no need for separate thread, we will wait for this anyway..
        //executor.addTask([playerUrl,prom]{
            
        auto downloader = HttpDownloader::create(HttpDownloader::upgradeToHttps(playerUrl));
        downloader->waitUntilFinished();
        if (downloader->failed()) {
            return shared_ptr<IJavascriptEngine>();
        }
        
        
        string playerJs = *downloader;
        auto engine = createEngine(playerJs);
        _currentEngine = std::make_pair(playerUrl, engine);
        return engine;

    }

    shared_ptr<IJavascriptEngine> method createEngine(const std::string &playerJs)
    {

        auto entry = entryFunction(playerJs);

        string processedJs = extractFunction(playerJs, entry) + "function decode(s){return " + entry + "(s);};";
#ifdef DEBUG
        {
            std::ofstream fs("/Users/zsszatmari/javascriptdebug0.txt");
            fs << playerJs << std::endl;
        }
        {
            std::ofstream fs("/Users/zsszatmari/javascriptdebug0-processed.js");
            fs << processedJs << std::endl;
        }
#endif   
        //std::cout << "player processed: " << processedJs << std::endl;
        
        
        auto engine = IApp::instance()->createJavascriptEngine();
        engine->load(processedJs);
                         
        return engine;
    }
}
