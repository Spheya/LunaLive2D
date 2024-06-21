#include "Parameter.hpp"

#include <string>
#include <Live2DCubismCore.h>

namespace luna {
	namespace live2d {

		Parameter::Parameter(const char* id, const float* minValue, const float* maxValue, const float* defaultValue, float* value) :
			m_id(id),
			m_minValue(minValue),
			m_maxValue(maxValue),
			m_defaultValue(defaultValue),
			m_value(value) {
			std::hash<std::string> hasher;
			m_hashId = hasher(std::string(m_id));
		}

		const char* Parameter::getId() const {
			return m_id;
		}

		size_t Parameter::getIdHash() const {
			return m_hashId;
		}

		float Parameter::getValue() const {
			return *m_value;
		}

		float Parameter::getDefaultValue() const {
			return *m_defaultValue;
		}

		float Parameter::getMinValue() const {
			return *m_minValue;
		}

		float Parameter::getMaxValue() const {
			return *m_maxValue;
		}

		void Parameter::setValue(float value) {
			*m_value = std::min(std::max(value, getMinValue()), getMaxValue());
		}

		float Parameter::getNormalizedValue() const {
			return getNormalizedValue(0.0f, -1.0f, 1.0f);
		}

		float Parameter::getNormalizedValue(float min, float max) const {
			return getNormalizedValue((min + max) * 0.5f, min, max);
		}

		float Parameter::getNormalizedValue(float base, float min, float max) const {
			if (min > max)
				std::swap(min, max);

			float middleValue = (getMinValue() + getMaxValue()) * 0.5f;
			float paramValue = getValue() - middleValue;

			if (paramValue < 0.0f)
				return (min - base) * paramValue / (getMinValue() - middleValue);
			return (max - base) * paramValue / (getMaxValue() - middleValue);
		}

	}
}