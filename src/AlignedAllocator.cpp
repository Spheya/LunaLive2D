#include "AlignedAllocator.hpp"

#include <stdlib.h>

// yoinked from https://github.com/Live2D/CubismNativeSamples/blob/develop/Samples/OpenGL/Demo/proj.win.cmake/src/LAppAllocator.cpp

namespace luna {
    namespace live2d {

        void* AlignedAllocator::allocate(size_t size, unsigned alignment) {
            size_t offset, shift, alignedAddress;
            void* allocation;
            void** preamble;

            offset = alignment - 1 + sizeof(void*);

            allocation = malloc(size + offset);

            alignedAddress = reinterpret_cast<size_t>(allocation) + sizeof(void*);

            shift = alignedAddress % alignment;

            if (shift) {
                alignedAddress += (alignment - shift);
            }

            preamble = reinterpret_cast<void**>(alignedAddress);
            preamble[-1] = allocation;

            return reinterpret_cast<void*>(alignedAddress);
        }

        void AlignedAllocator::deallocate(void* data) {
            if (data == nullptr) return;

            void** preamble;

            preamble = static_cast<void**>(data);

            free(preamble[-1]);
        }

    }
}
