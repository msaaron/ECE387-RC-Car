cmake_minimum_required(VERSION 3.12)

# Must be before project
include(../cmake/pico_sdk_import.cmake)

project(rc_car_final C CXX ASM)

# Initialize the Pico SDK
pico_sdk_init()

# Include RF24 CMakeLists.txt
include(../lib/RF24/CMakeLists.txt)

# iterate over a list of examples by name
set(COMPONENTS_LIST
    car
)

foreach(component ${COMPONENTS_LIST})
    # make a target
    add_executable(${component} ${component}.cpp)

    # link the necessary libs to the target
    target_link_libraries(${component} PUBLIC
        RF24
        pico_stdlib
        hardware_spi
        hardware_gpio
        hardware_pwm
    )

    # create map/bin/hex file etc.
    pico_add_extra_outputs(${component})
endforeach()
