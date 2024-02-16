set(TARGET AQARA_D1_ALT)

file(GLOB SDK_INCLUDE_FOLDERS "${SDK_COMPONENTS_BASE_DIR}/**/Include")
include_directories(${SDK_INCLUDE_FOLDERS})

link_directories(
    ${SDK_COMPONENTS_BASE_DIR}/Library
    ${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -TAppBuildZBPro.ld")

set(SDK_SRC    
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZQueue.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZTimer.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/app_zps_link_keys.c
)
add_library(ZigBee STATIC ${SDK_SRC})

set(APPLIBS
    ZPSMAC_Mini_SOC 
    PWRM 
    ZPSTSV
    AES_SW
    PDUM
    ZPSAPL
    Random
    PDM_EEPROM_NO_RTOS
    DBG
    ZPSNWK_ZED
    MiniMac
    MiniMacShim
    MMAC
    Aes
    HardwareApi
    MicroSpecific
    Boot
    JPT
)
set(LDLIBS)
foreach(lib ${APPLIBS})
    set(my_file_path "${SDK_COMPONENTS_BASE_DIR}/Library/lib${lib}_${JENNIC_CHIP}.a")
    if (EXISTS ${my_file_path})
        list(APPEND LDLIBS ${lib}_${JENNIC_CHIP})
    else()
        list(APPEND LDLIBS ${lib}_${JENNIC_CHIP_FAMILY})
    endif()
endforeach()

set(APP_SRC
    main.c
)
add_executable(${TARGET} ${APP_SRC})
set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

target_link_libraries(${TARGET}
    -Wl,--start-group
    ZigBee
    ${LDLIBS}
    -Wl,--end-group
)

add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${TARGET}.elf)

add_custom_target(${TARGET}.bin
    DEPENDS ${TARGET}
    COMMAND ${CMAKE_COMMAND} -E echo "Generating binary ..."
    COMMAND ${CMAKE_OBJCOPY} -j .version -j .bir -j .flashheader -j .vsr_table -j .vsr_handlers  -j .rodata -j .text -j .data -j .bss -j .heap -j .stack -S -O binary ${TARGET}.elf ${TARGET}.bin
)
