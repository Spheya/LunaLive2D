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
			*m_value = value;
		}

	}
}