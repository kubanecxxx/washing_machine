cmake_minimum_required(VERSION 3.4)

project(pracka)


#todo
#cmakefiles for my modules


set (USER_SOURCES 
main.cpp
relays.c
display.c
LCD_ctrl.c
rotary_encoder.c
inputs.c
statemachine.cpp
gui.cpp
)

set (EXTRA_HEADERS
halconf.h
chconf.h
mcuconf.h
platform.h
board_f100/board.h

)

set (OPTIMIZED_SOURCES
    chsprintf.c
    board_f100/board.c
)

include_directories(
board_f100
)

set (INCLUDE_DIRS

)

set (USER_PROGRAM_START 0x08002800)
set (USE_BOOTLOADER 0)

#set (LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F100xB.ld")

set (MAIN_STACK_SIZE 0x400)
set (PROCESS_STACK_SIZE 0x400)
set (OPTIMIZE_RTOS 0)

set (GLOBAL_FLAGS
"-O0  -ggdb "
)

set (OPTIMIZED_FLAGS
"-O2"
)

set (CFLAGS

)

set (CPPFLAGS
"-fno-rtti -fno-exceptions "
)

#family or chip
set (CHIP "STM32F100C6")
#set (CHIP "STM32F407VG")
#chibios from https://github.com/kubanecxxx/chibios_fork commit hash
#4e1e344475204f1f255eb6fe78c55527988e502a
set (CHIBIOS_DIR "/home/kubanec/workspaces/chibios_3stable")
if (NOT EXISTS ${CHIBIOS_DIR})
	set (CHIBIOS_DIR "/home/kuba/chibi/chibios_fork")
endif()
#set (BOARD "ST_STM32F4_DISCOVERY")

set (FLASH_SIZE "32k")
set (RAM_SIZE "4k")

set (TOOLCHAIN "/home/kubanec/toolchain/4_9-2015q2/bin/arm-none-eabi")


set(USE_COMMON_LIBRARY 1)
set(LIBRARY_MODULES scheduler)

set(USE_PIRIS 0)

include (${CHIBIOS_DIR}/CMakeLists.txt)
