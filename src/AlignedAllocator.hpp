#pragma once

namespace luna {
	namespace live2d {

		class AlignedAllocator {
		public:
			static void* allocate(size_t size, unsigned alignment);
			static void deallocate(void* data);

		private:
			AlignedAllocator() = default;
		};

	}
}
