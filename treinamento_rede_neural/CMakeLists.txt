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
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(treinamento_rede_neural C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(treinamento_rede_neural treinamento_rede_neural.c )

pico_set_program_name(treinamento_rede_neural "treinamento_rede_neural")
pico_set_program_version(treinamento_rede_neural "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(treinamento_rede_neural 0)
pico_enable_stdio_usb(treinamento_rede_neural 0)
add_library(kissfft STATIC kiss_fft.c kiss_fftr.c)
add_library(cjson cJSON.c) 


# Add the standard library to the build
target_link_libraries(treinamento_rede_neural
        pico_stdlib
        cjson)


# Add the standard include files to the build
target_include_directories(treinamento_rede_neural PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(treinamento_rede_neural 
        pico_cyw43_arch_lwip_poll
        hardware_adc
        hardware_dma
        kissfft
        cJSON

        )

pico_add_extra_outputs(treinamento_rede_neural)

