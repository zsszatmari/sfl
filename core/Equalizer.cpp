#include <iostream>
#include <sstream>
#include <cstdio>
#include "Equalizer.h"
extern "C" {
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}
#include "IApp.h"
#include "IPreferences.h"
#include "json.h"

#if (LIBAVUTIL_VERSION_MAJOR != 54) || (LIBAVUTIL_VERSION_MINOR != 15)
#error incorrect ffmpeg version
#endif

namespace Gear
{
#define method Equalizer::

    using std::vector;

	static int registerAll()
	{
		avfilter_register_all();
		return 0;
	}

	method Equalizer() :
		_filterGraph(nullptr),
		_filterIn(nullptr),
		_filterOut(nullptr),
		_filterPre(nullptr),
        //_filterWiden(nullptr),
        _widen(IApp::instance()->preferences().boolForKey("eqWiden"))
	{
        assert(avutil_version() == LIBAVUTIL_VERSION_INT);

        _changed = false;
#ifdef DEBUG
		std::cout << "creating equalizer...\n";
#endif
		static auto reg = registerAll();

        _workFrame = av_frame_alloc();

	
        _abuffer = avfilter_get_by_name("abuffer");
        _volume = avfilter_get_by_name("volume");
        _equalizer = avfilter_get_by_name("equalizer");
        _abuffersink = avfilter_get_by_name("abuffersink");
        _aformat = avfilter_get_by_name("aformat");
        //_aearwax = avfilter_get_by_name("earwax");

        if (!_abuffer || !_volume || !_equalizer || !_abuffersink || !_aformat /*|| !_aearwax*/) {
        	std::cout << "filter not found\n";
            return;
        }

        _parameters = fetchParameters();
        if (resetNodes()) {
#ifdef DEBUG
    		std::cout << "creating equalizer... success!\n";
#endif
        }
    }

    void method freeGraph()
    {
        avfilter_free(_filterIn); _filterIn = nullptr;
        avfilter_free(_filterPre); _filterPre = nullptr;
        avfilter_free(_filterOut); _filterOut = nullptr;
        //avfilter_free(_filterWiden); _filterWiden = nullptr;
        for (AVFilterContext *filter : _filterEq) {
            avfilter_free(filter);
        }
        _filterEq.resize(0);
        avfilter_graph_free(&_filterGraph);
    }

    bool method resetNodes()
    {
#ifdef _WIN32
#define snprintf _snprintf
#endif

        freeGraph();

        _filterGraph = avfilter_graph_alloc();
        char strbuf[512];
        snprintf(strbuf, sizeof(strbuf), "sample_fmt=%d:sample_rate=44100:channels=2:channel_layout=0x%d", AV_SAMPLE_FMT_FLT, AV_CH_LAYOUT_STEREO);
        if (avfilter_graph_create_filter(&_filterIn, _abuffer, NULL, strbuf, NULL, _filterGraph) < 0) {
            return false;
        }
        
        // _parameters.pre
        // dB = 20 * log10(volumeratio) = 20 * ln(volumeratio)/ln(10)
        // exp(db/20 * ln(10)) = volumeratio
        auto volumeRatio = expf(_parameters.pre / 20 * logf(10));
        snprintf(strbuf, sizeof(strbuf), "volume=%f", (float)volumeRatio);
        //std::cout << "volume filter: '" << strbuf << "'\n";
        if (avfilter_graph_create_filter(&_filterPre, _volume, NULL, strbuf, NULL, _filterGraph) < 0) {
            return false;
        }

        if (avfilter_graph_create_filter(&_filterOut, _abuffersink, NULL, NULL, NULL, _filterGraph) < 0) {
            return false;
        }

        snprintf(strbuf, sizeof(strbuf), "sample_fmts=flt:channel_layouts=stereo:sample_rates=44100");
        if (avfilter_graph_create_filter(&_filterFormat, _aformat, NULL, strbuf, NULL, _filterGraph) < 0) {
            return false;
        }

        // http://sound.stackexchange.com/questions/14101/the-formula-for-10-and-31-multi-band-eq-frequency-ranges
        const float freqs[] = {31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
        _filterEq = std::vector<AVFilterContext *>(_parameters.channels.size(), nullptr);
        for (int i = 0 ; i < _parameters.channels.size() ; ++i) {
            const float freq = freqs[i]; 
            snprintf(strbuf, sizeof(strbuf), "frequency=%f:width_type=o:width=1:gain=%f", freq, (float)_parameters.channels[i]);
            if (avfilter_graph_create_filter(&_filterEq[i], _equalizer, NULL, strbuf, NULL, _filterGraph) < 0) {
                return false;
            }
        }

        std::vector<AVFilterContext *> chain;
        chain.push_back(_filterIn);
        chain.push_back(_filterPre);
        for (int i = 0 ; i < _parameters.channels.size() ; ++i) {
            chain.push_back(_filterEq[i]);
        }

        // this is crap, we do something else instead
        /*if (IApp::instance()->preferences().boolForKey("eqWiden")) {
            if (avfilter_graph_create_filter(&_filterWiden, _aearwax, NULL, NULL, NULL, _filterGraph) < 0) {
                std::cout << "error applying widening effect\n";
                return false;
            }
            chain.push_back(_filterWiden);
        }*/
        chain.push_back(_filterFormat);
        chain.push_back(_filterOut);

        for (auto it = chain.begin() + 1; it != chain.end() ; ++it) {
            if (avfilter_link(*(it-1), 0, *it, 0) < 0) {
                return false;
            }
        }

        if (avfilter_graph_config(_filterGraph, NULL) < 0) {
            return false;
        }

        return true;
    }

    std::vector<std::string> method bandLabels() const
    {
        return std::vector<std::string>{"32","64","125","250","500","1k","2k","4k","8k","16k"};
    }

    Equalizer::Parameters method fetchParameters()
    {
        Parameters param;
        const auto &prefs = IApp::instance()->preferences();
        param.on = prefs.boolForKey("eqEnabled");
        if (!param.on) {
            return param;
        }
        param.pre = prefs.floatForKey("eqPre");
        param.channels.resize(10);
        for (int i = 0 ; i < param.channels.size() ; ++i) {
            std::stringstream ss;
            ss << i;
            param.channels[i] = prefs.floatForKey(std::string("eq") + ss.str());
        }

        return param;
    }

    void method setParameters(const Parameters &params)
    {
#ifdef DEBUG
        //std::cout << "setting " << (std::string)params << std::endl;
#endif

        auto &prefs = IApp::instance()->preferences();
        prefs.setUintForKey("eqEnabled", params.on);
        prefs.setFloatForKey("eqPre", params.pre);

        auto channels = params.channels;
        while (channels.size() < 10) {
            channels.push_back(0);
        }
        for (int i = 0 ; i < channels.size() ; ++i) {
            std::stringstream ss;
            ss << i;
            prefs.setFloatForKey(std::string("eq") + ss.str(), channels[i]);
        }
        notifyChange();
    }

    void method notifyChange(bool legacy) 
    {
        // this should only be called if there is a real change
        _changed = true;
        if (!legacy) {
            IApp::instance()->preferences().setUintForKey("eqModified", true);
        }
    }

    void method setWiden(bool on)
    {
        /*
        _changed = true;*/
        IApp::instance()->preferences().setUintForKey("eqWiden", on);
        _widen = on;
    }

	void method process(char *data, size_t num)
	{
        if (num == 0) {
            return;
        }

        static uint8_t counter = 0;
        counter += 8;       // for 128 byte packets, this means relatively smooth transitions
        if (counter == 0) {
            bool expect = true;
            if (_changed.compare_exchange_strong(expect, false)) {
                // it would slow us if we refetched settings everytime, and there is a strange buzzing noise
                auto newParameters = fetchParameters();
                if (newParameters != _parameters) {
                    _parameters = newParameters;
                    if (!resetNodes()) {
                        _parameters.reset();
                        return;
                    }
                }
            }
        }

        if (_parameters.on) {

            //char errstr[512];
            
    		_workFrame->format = AV_SAMPLE_FMT_FLT;
    		_workFrame->nb_samples = num / (2*sizeof(float));
            _workFrame->linesize[0] = num;
    		_workFrame->channels = 2;
            _workFrame->sample_rate = 44100;
            _workFrame->channel_layout = AV_CH_LAYOUT_STEREO;
    		_workFrame->data[0] = reinterpret_cast<uint8_t *>(data);

            // see https://libav.org/doxygen/master/group__lavfi__buffersrc.html on ownership
    		int err = av_buffersrc_add_frame(_filterIn, _workFrame);
    		//std::cout << "processing push: " << err << " " << " samples: " << frame->nb_samples << " " << av_make_error_string(errstr,sizeof(errstr),err) << std::endl;
            //av_frame_unref(frame); TODO?
            
            _workFrame->format = AV_SAMPLE_FMT_FLT;
            _workFrame->nb_samples = num / (2*sizeof(float));
            _workFrame->linesize[0] = num;
            _workFrame->channels = 2;
            _workFrame->sample_rate = 44100;
            _workFrame->channel_layout = AV_CH_LAYOUT_STEREO;
            _workFrame->data[0] = NULL;
            // less efficient, but more controlled:
            //err = av_buffersink_get_samples(_filterOut, frame, frame->nb_samples);
            err = av_buffersink_get_frame(_filterOut, _workFrame);
            if (err >= 0) {
                std::copy(_workFrame->data[0], _workFrame->data[0] + num, data);
            	
                //memcpy(data, frame->data[0], num);
            }
            assert(_workFrame->format == AV_SAMPLE_FMT_FLT);

            //std::cout << "processing pull: " << err << " samples: " << frame->nb_samples << std::endl;
            
            av_frame_unref(_workFrame);
        }

        if (_widen) {
            float *floatData = reinterpret_cast<float *>(data);
            for (int i = 0 ; i < num/sizeof(float)/2 ; i++) {
                //http://www.musicdsp.org/archive.php?classid=4#173

                const float WideCoeff = 1.0f;

                float &SamplL = floatData[2*i +0];
                float &SamplR = floatData[2*i +1];
                float MonoSign = (SamplL + SamplR)/2.0;
                float DeltaLeft = (SamplL - MonoSign) * WideCoeff;
                SamplL = SamplL + DeltaLeft;
                SamplR = SamplR - DeltaLeft;  
            }
        }
	}			

	method ~Equalizer()
	{
		freeGraph();
        av_frame_free(&_workFrame);
	}

    static Json::Value presets()
    {
        auto str = IApp::instance()->preferences().stringForKey("EqualizerPresets");
        Json::Value ret;
        Json::Reader reader;
        reader.parse(str, ret);
        if (!ret.isArray()) {
            ret = Json::arrayValue;
        }
        if (ret.empty()) {
            Json::Value flat = Json::objectValue;
            flat["name"] = "Flat";
            ret.append(flat);
        }
        return ret;
    }

    static void writePresets(const Json::Value &value)
    {
        Json::FastWriter writer;
        IApp::instance()->preferences().setStringForKey("EqualizerPresets", writer.write(value));
    }

    vector<std::string> method queryPresetNames() const
    {
        Json::Value value = presets();
        std::vector<std::string> names;
        if (value.isArray()) {
            for (int i = 0 ; i < value.size() ; ++i) {
                string name = value[i].get("name","unknown").asString();
                if (value[i].get("current",false).asBool() && IApp::instance()->preferences().boolForKey("eqModified")) {
                    name = std::string("*") + name;
                }
                names.push_back(name);
            }
        }
        if (names.empty()) {
            names.push_back("Flat");
        }
        return names;
    }

    int method currentPreset() const
    {
        Json::Value value = presets();
        if (value.isArray()) {
            for (int i = 0 ; i < value.size() ; ++i) {
                if (value[i].get("current",false).asBool()) {
                    return i;
                }
            }
        }
        return 0;
    }

    bool method currentModifiable() const
    {
        return currentPreset() != 0;
    }

    bool method currentModified() const
    {
        return IApp::instance()->preferences().uintForKey("eqModified");
    }

    void method selectPreset(size_t index)
    {
        Json::Value value = presets();
        if (value.isArray()) {
            for (int i = 0 ; i < value.size() ; ++i) {
                value[i]["current"] = (index == i);
                if (index == i) {
                    // set eq data according to saved settings
                    setParameters(parameters(value[i]));
                    IApp::instance()->preferences().setUintForKey("eqModified", false);
                    if (index == 0) {
                        IApp::instance()->preferences().setUintForKey("eqEnabled", false);
                    }
                }
            }
        }
        writePresets(value);
    }

    Equalizer::Parameters method parameters(const Json::Value &params)
    {
        Equalizer::Parameters ret;
        ret.on = true;
        ret.pre = params.get("pre",0.0f).asFloat();
        auto channels = params.get("channels",Json::arrayValue);
        if (channels.isArray()) {
            ret.channels.resize(channels.size());
            for (int i = 0 ; i < channels.size() ; ++i) {
                ret.channels[i] = channels[i].asFloat();
            }
        }
        return ret;
    }

    void method saveCurrentPresetOverwrite() const
    {
        auto allPresets = presets();
        int current = currentPreset();
        if (current >= allPresets.size()) {
            return;
        }

        Json::Value &toSave = allPresets[current];
        auto params = fetchParameters();
        Json::Value channels(Json::arrayValue);
        for (int i = 0 ; i < params.channels.size() ; ++i) {
            channels.append(params.channels[i]);
        }
        toSave["channels"] = channels;
        toSave["pre"] = params.pre;

        writePresets(allPresets);

        IApp::instance()->preferences().setUintForKey("eqModified", false);
    }    

    void method saveCurrentPreset(const std::string &name)
    {
        auto allPresets = presets();
        Json::Value toSave(Json::objectValue);
        toSave["name"] = name;
        
        auto params = fetchParameters();
        Json::Value channels(Json::arrayValue);
        for (int i = 0 ; i < params.channels.size() ; ++i) {
            channels.append(params.channels[i]);
        }

        toSave["channels"] = channels;
        toSave["pre"] = params.pre;
        allPresets.append(toSave);
        for (int i = 0 ; i < allPresets.size() ; ++i) {
            allPresets[i]["current"] = (i == allPresets.size()-1);
        }

        writePresets(allPresets);

        IApp::instance()->preferences().setUintForKey("eqModified", false);
    }

    void method deleteCurrentPreset()
    {
        int current = currentPreset();
        auto allPresetsOld = presets();
        Json::Value allPresets(Json::arrayValue);

        for (int i = 0 ; i < allPresetsOld.size() ; ++i) {
            if (i != current) {
                allPresets.append(allPresetsOld[i]);
            }
        }

        writePresets(allPresets);

        // this is awkward:
        // we can view what left remaining as a 'modified' version of the other (possibly the 'Flat') preset
        //IApp::instance()->preferences().setUintForKey("eqModified", true);

        // instead:
        selectPreset(0);
    }

#undef method
#define method Equalizer::Parameters::

    method Parameters()
    {
        reset();
    }

    void method reset()
    {
        memset(this, 0, sizeof(Parameters));
    }

    bool method operator==(const Equalizer::Parameters &rhs) const
    {
        return memcmp(this, &rhs, sizeof(Parameters)) == 0;
    }

    bool method operator!=(const Equalizer::Parameters &rhs) const
    {
        return !(*this == rhs);
    }

    method operator std::string() const
    {
        std::stringstream ss;
        ss << "pre: " << pre << " bands:";
        for (auto band : channels) {
            ss << band << " ";
        }
        return ss.str();
    }


#undef method
}