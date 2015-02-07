#ifndef IEQUALIZER_H
#define IEQUALIZER_H

#include <cstddef>
#include <vector>
#include <string>

namespace Gear
{
	class IEqualizer
	{
	public:
		virtual void process(char *data, size_t num) = 0;
		virtual void notifyChange(bool legacy = false) = 0;

		virtual std::vector<std::string> queryPresetNames() const = 0;
		virtual int currentPreset() const = 0;
		virtual void selectPreset(size_t index) = 0;
		virtual void saveCurrentPresetOverwrite() const = 0;
		virtual void saveCurrentPreset(const std::string &name) = 0;
		virtual void deleteCurrentPreset() = 0;
		virtual bool currentModifiable() const = 0;


		virtual ~IEqualizer();
	};
}

#endif