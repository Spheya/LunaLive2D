#pragma once

namespace luna {
	namespace live2d {

		class Parameter {
		public:
			Parameter(const char* id, const float* minValue, const float* maxValue, const float* defaultValue, float* value);
			Parameter(Parameter&) = delete;
			Parameter& operator=(Parameter&) = delete;
			Parameter(Parameter&&) noexcept = default;
			Parameter& operator=(Parameter&&) noexcept = default;
			~Parameter() = default;

			const char* getId() const;
			size_t getIdHash() const;

			float getValue() const;
			float getDefaultValue() const;
			float getMinValue() const;
			float getMaxValue() const;

			void setValue(float value);

		private:

		private:
			size_t m_hashId;

			const char* m_id;
			const float* m_minValue;
			const float* m_maxValue;
			const float* m_defaultValue;

			float* m_value;
		};

	}
}