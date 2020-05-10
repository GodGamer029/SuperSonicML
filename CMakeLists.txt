# 3.16.6 adds precompiled header suppor
# if you want to use an older version, remove target_precompiled_headers
cmake_minimum_required(VERSION 3.16.6)

project(SuperSonicML)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT CMAKE_BUILD_TYPE MATCHES Release)
    message(FATAL_ERROR "Bakkesmod plugins have to be built in release mode to work correctly, use -DCMAKE_BUILD_TYPE=Release, current mode: \"${CMAKE_BUILD_TYPE}\" config: ${CONFIG}")
endif()

find_package(Torch)

if(NOT Torch_FOUND)
    message(FATAL_ERROR "Torch not found in module path, add -DCMAKE_PREFIX_PATH=C:/path/to/libtorch to your commandline")
endif()

if(NOT MSVC)
    message(WARNING "This build script was only tested with MSVC")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} /O2")

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX WIN32 SuperSonicML_EXPORTS _WINDOWS _USRDLL)

include_directories(.)
include_directories(include)
include_directories(src)

file(GLOB_RECURSE includes RELATIVE include/
        "*.h"
        "*.cpp"
        "*.hpp"
        )

link_directories(include/bakkesmod/lib)

add_library(SuperSonicML SHARED
        ${includes}
        math/mat.h
        math/math.h
        math/vec.h
        src/Constants.h
        src/GameData.cpp
        src/GameData.h
        src/Hooks.cpp
        src/Hooks.h
        src/SuperSonicML.cpp
        src/SuperSonicML.h src/bots/BotClass.cpp src/bots/BotClass.h src/bots/AtbaBot.cpp src/bots/AtbaBot.h src/bots/AerialAtbaBot.cpp src/bots/AerialAtbaBot.h src/experiments/Experiment.cpp src/experiments/Experiment.h src/experiments/TeacherLearnerExperiment.cpp src/experiments/TeacherLearnerExperiment.h)

target_precompile_headers(SuperSonicML
        PRIVATE
            <torch/torch.h>
#            <math/math.h>
        )
target_link_libraries(SuperSonicML "${TORCH_LIBRARIES}")
target_compile_features(SuperSonicML PUBLIC cxx_std_17)

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET SuperSonicML
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TORCH_DLLS}
            $<TARGET_FILE_DIR:SuperSonicML>)

    foreach (flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        # remove /EHsc because torch defines /EHa
        string(REPLACE "/EHsc" "" ${flag_var} "${${flag_var}}")
    endforeach()
endif (MSVC)