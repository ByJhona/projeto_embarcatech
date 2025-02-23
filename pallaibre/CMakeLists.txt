# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# ================================================
set(PICO_SDK_PATH "/mnt/c/Users/Jhon/.pico-sdk/sdk/2.1.0")

include_directories("${PICO_SDK_PATH}/src/boards/include")
include_directories("${PICO_SDK_PATH}/src/common/pico_base_headers/include")
include_directories("${PICO_SDK_PATH}/src/common/pico_stdlib_headers/include")
set(PROJECT_FOLDER .)

include(pico_sdk_import.cmake)

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

pico_sdk_init()

add_definitions(-DEIDSP_QUANTIZE_FILTERBANK=0
-DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=0
-DARM_MATH_LOOPUNROLL
)

add_executable(app
source/main.cpp
)

pico_generate_pio_header(app ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)
include(${PROJECT_FOLDER}/edge-impulse-sdk/cmake/utils.cmake)

target_link_libraries(app pico_stdlib
    hardware_adc
    hardware_dma
    hardware_pio
    hardware_clocks)



target_include_directories(app PRIVATE
    ${PROJECT_FOLDER}/tflite-model
    ${PROJECT_FOLDER}/model-parameters
    ${PROJECT_FOLDER}
    ${CMAKE_CURRENT_LIST_DIR}


)

# enable usb output, disable uart output
pico_enable_stdio_usb(app 1)
pico_enable_stdio_uart(app 0)

RECURSIVE_FIND_FILE(SOURCE_FILES "${PROJECT_FOLDER}/edge-impulse-sdk" "*.cpp")
RECURSIVE_FIND_FILE(MODEL_FILES "${PROJECT_FOLDER}/tflite-model" "*.cpp")
RECURSIVE_FIND_FILE(CC_FILES "${PROJECT_FOLDER}/edge-impulse-sdk" "*.cc")
RECURSIVE_FIND_FILE(C_FILES "${PROJECT_FOLDER}/edge-impulse-sdk" "*.c")

list(APPEND SOURCE_FILES ${S_FILES})
list(APPEND SOURCE_FILES ${C_FILES})
list(APPEND SOURCE_FILES ${CC_FILES})
list(APPEND SOURCE_FILES ${MODEL_FILES})

# add all sources to the project
target_sources(app PRIVATE ${SOURCE_FILES})

pico_add_extra_outputs(app)
