set(APP_TARGET FIRMWARE)
set(SDK_TARGET SDK)

set(JENNIC_CHIP JN5169)
set(JENNIC_CHIP_FAMILY JN516x)

add_compile_definitions(
    -DJN516x=5160
    -DJN5169=5169
    -DJENNIC_CHIP_NAME=_${JENNIC_CHIP}
    -DJENNIC_CHIP_FAMILY_NAME=_${JENNIC_CHIP_FAMILY}

    -DJENNIC_CHIP=${JENNIC_CHIP}
    -DJENNIC_CHIP_${JENNIC_CHIP}
    -DJENNIC_CHIP_FAMILY=${JENNIC_CHIP_FAMILY}
    -DJENNIC_CHIP_FAMILY_${JENNIC_CHIP_FAMILY}

    -DDBG_ENABLE
    -DEMBEDDED
    -DPDM_NO_RTOS
    -DJENNIC_MAC_MiniMacShim
    -DJENNIC_STACK_ZBPro
)

set(SDK_PREFIX ${CMAKE_SOURCE_DIR}/sdk)
set(SDK_COMPONENTS_BASE_DIR ${SDK_PREFIX}/Components)

file(GLOB SDK_INCLUDE_FOLDERS "${SDK_COMPONENTS_BASE_DIR}/**/Include")
include_directories(${SDK_INCLUDE_FOLDERS})

link_directories(
    ${SDK_COMPONENTS_BASE_DIR}/Library
    ${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build/AppBuildZBPro.ld")

# ============= START - SDK library ============= 
set(SDK_SRC
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZQueue.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZTimer.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/app_zps_link_keys.c
)
add_library(${SDK_TARGET} STATIC ${SDK_SRC})

set(LDLIBS
    MiniMac_${JENNIC_CHIP}
    MiniMacShim_${JENNIC_CHIP_FAMILY}
    MMAC_${JENNIC_CHIP}
    JPT_${JENNIC_CHIP}
    Aes_${JENNIC_CHIP_FAMILY}
    HardwareApi_${JENNIC_CHIP}
    MicroSpecific_${JENNIC_CHIP_FAMILY}
    Boot_${JENNIC_CHIP_FAMILY}
    PWRM_${JENNIC_CHIP_FAMILY}
    ZPSTSV_${JENNIC_CHIP_FAMILY}
    AES_SW_${JENNIC_CHIP_FAMILY}
    PDUM_${JENNIC_CHIP_FAMILY}
    ZPSAPL_${JENNIC_CHIP_FAMILY}
    Random_${JENNIC_CHIP_FAMILY}
    PDM_EEPROM_NO_RTOS_${JENNIC_CHIP_FAMILY}
    ZPSNWK_ZED_${JENNIC_CHIP_FAMILY}
    DBG_${JENNIC_CHIP_FAMILY}
)
# ============= END - SDK library ============= 

# ============= START - Firmware ============= 
set(APP_SRC
    main.c
)
add_executable(${APP_TARGET} ${APP_SRC})

target_link_libraries(${APP_TARGET}
    -Wl,--start-group
    ${SDK_TARGET}
    ${LDLIBS}
    -Wl,--end-group
)

set_target_properties(${APP_TARGET} PROPERTIES SUFFIX ".elf")
add_custom_command(TARGET ${APP_TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${APP_TARGET}.elf)
add_custom_target(${APP_TARGET}.bin ALL
    DEPENDS ${APP_TARGET}
    COMMAND ${CMAKE_COMMAND} -E echo "Generating binary ..."
    COMMAND ${CMAKE_OBJCOPY} -j .version -j .bir -j .flashheader -j .vsr_table -j .vsr_handlers  -j .rodata -j .text -j .data -j .bss -j .heap -j .stack -S -O binary ${APP_TARGET}.elf ${APP_TARGET}.bin
)
# ============= END - Firmware ============= 
