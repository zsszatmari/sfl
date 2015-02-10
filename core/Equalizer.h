#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <vector>
#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavfilter/avfilter.h>
}
#include "IEqualizer.h"
#include "stdplus.h"
#include "json-forwards.h"
#include ATOMIC_H

namespace Gear
{
	class Equalizer final : public IEqualizer
	{
	public:
		Equalizer();
		virtual void process(char *data, size_t num);
		virtual void notifyChange(bool legacy = false);
		virtual ~Equalizer();

		virtual std::vector<std::string> queryPresetNames() const override;
		virtual int currentPreset() const override;
		virtual void selectPreset(size_t index) override;
		virtual bool currentModifiable() const override;
		virtual void saveCurrentPresetOverwrite() const override;
		virtual void saveCurrentPreset(const std::string &name) override;
		virtual void deleteCurrentPreset() override;

	private:
		struct Parameters final {
			Parameters();
			void reset();

			bool on;
			float pre;
			std::vector<float> channels;

			bool operator==(const Parameters &rhs) const;
			bool operator!=(const Parameters &rhs) const;
		};

		Parameters _parameters;
		bool resetNodes();
		void freeGraph();
		static Parameters fetchParameters();
		static Parameters parameters(const Json::Value &params);
		void setParameters(const Parameters &params);
		//static Json::Value jsonParameters();

		AVFilter * _abuffer;
        AVFilter * _volume;
        AVFilter * _equalizer;
        AVFilter * _aformat;
        AVFilter * _abuffersink;

        ATOMIC_NS::atomic_bool _changed;

        AVFrame *_workFrame;

        AVFilterGraph *_filterGraph;
        AVFilterContext *_filterIn;
        AVFilterContext *_filterPre;
        std::vector<AVFilterContext *> _filterEq;
        AVFilterContext *_filterFormat;
        AVFilterContext *_filterOut;
	};
}

#endif