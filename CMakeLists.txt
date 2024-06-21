cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_subdirectory ("Luna")

project ("LunaLive2D")

# Detect architecture.
if(CMAKE_EXE_LINKER_FLAGS STREQUAL "/machine:x64")
  set(ARCH x86_64)
elseif(CMAKE_EXE_LINKER_FLAGS STREQUAL "/machine:X86")
  set(ARCH x86)
else()
  message(FATAL_ERROR "[${APP_NAME}] Invalid linker flag ${CMAKE_EXE_LINKER_FLAGS}.")
endif()
# Detect compiler.
if(MSVC_VERSION MATCHES 1800)
  # Visual Studio 2013
  set(COMPILER 120)
elseif(MSVC_VERSION MATCHES 1900)
  # Visual Studio 2015
  set(COMPILER 140)
elseif(MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS 1920)
  # Visual Studio 2017
  set(COMPILER 141)
elseif(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS 1930)
  # Visual Studio 2019
  set(COMPILER 142)
elseif(MSVC_VERSION GREATER_EQUAL 1930 AND MSVC_VERSION LESS 1940)
  # Visual Studio 2022
  set(COMPILER 143)
elseif(MSVC)
  message(FATAL_ERROR "[${APP_NAME}] Unsupported Visual C++ compiler used.")
else()
  message(FATAL_ERROR "[${APP_NAME}] Unsupported compiler used.")
endif()
# Detect core crt.
if(CORE_CRL_MD)
  set(CRT MD)
else()
  set(CRT MT)
endif()

add_library(Live2DCubismCore STATIC IMPORTED GLOBAL)
set_target_properties(Live2DCubismCore
  PROPERTIES
    IMPORTED_LOCATION_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/Core/lib/windows/${ARCH}/${COMPILER}/Live2DCubismCore_${CRT}d.lib
    IMPORTED_LOCATION_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/Core/lib/windows/${ARCH}/${COMPILER}/Live2DCubismCore_${CRT}.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/Core/include
)

add_library(json INTERFACE)
target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/json/single_include)

set(SOURCE_FILES 
	"src/AlignedAllocator.cpp"
	"src/Drawable.cpp"
	"src/Model.cpp"
	"src/ModelInstance.cpp"
	"src/Parameter.cpp"
	"src/Physics.cpp"
	"src/Renderer.cpp"
)

set(INCLUDE_FILES 
	"src/Drawable.hpp"
	"src/LunaLive2D.hpp"
	"src/ModelInstance.hpp"
	"src/Model.hpp"
	"src/Parameter.hpp"
	"src/Pysics.hpp"
	"src/Renderer.hpp"
)

add_library(lunalive2d $<TARGET_OBJECTS:luna> ${SOURCE_FILES})
set_property(TARGET lunalive2d PROPERTY CXX_STANDARD 20)

target_include_directories(lunalive2d PUBLIC "src")
target_link_libraries(lunalive2d PUBLIC luna Live2DCubismCore)
target_link_libraries(lunalive2d PRIVATE json)

install(TARGETS lunalive2d DESTINATION lib)
install(FILES Core/include/Live2DCubismCore.h DESTINATION include)
install(FILES ${INCLUDE_FILES} DESTINATION include)
install(FILES LICENSE DESTINATION .)
install(FILES Core/LICENSE.md DESTINATION .)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_subdirectory ("example")
endif()