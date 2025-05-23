set(APP_TARGET ${TARGET})
set(SDK_TARGET SDK)
set(VENV_DIR $ENV{VENV_DIR})
set(BUILD_NUMBER "000001")
string(TIMESTAMP BUILD_DATE "%Y%m%d" UTC)

set(JENNIC_CHIP JN5169)
set(JENNIC_CHIP_FAMILY JN516x)

add_compile_definitions(
    JN516x=5160
    JN5169=5169
    JENNIC_CHIP_NAME=_${JENNIC_CHIP}
    JENNIC_CHIP_FAMILY_NAME=_${JENNIC_CHIP_FAMILY}

    JENNIC_CHIP=${JENNIC_CHIP}
    JENNIC_CHIP_${JENNIC_CHIP}
    JENNIC_CHIP_FAMILY=${JENNIC_CHIP_FAMILY}
    JENNIC_CHIP_FAMILY_${JENNIC_CHIP_FAMILY}

    EMBEDDED
    PDM_NO_RTOS
    JENNIC_MAC_MiniMacShim
    JENNIC_STACK_ZBPro
    BDB_SUPPORT_NWK_STEERING
    ZBPRO_DEVICE_TYPE_ZED

    # DBG_ENABLE
    # DEBUG_BDB
    BUILD_DATE="${BUILD_DATE}"
    BUILD_NUMBER="${BUILD_NUMBER}"
)

set(SDK_PREFIX ${PROJECT_SOURCE_DIR}/sdk)
set(SDK_COMPONENTS_BASE_DIR ${SDK_PREFIX}/Components)

set(SDK_INCLUDE_FOLDERS
    ${SDK_COMPONENTS_BASE_DIR}/Aes/Include
    ${SDK_COMPONENTS_BASE_DIR}/AES_SW/Include
    ${SDK_COMPONENTS_BASE_DIR}/Common/Include
    ${SDK_COMPONENTS_BASE_DIR}/DBG/Include
    ${SDK_COMPONENTS_BASE_DIR}/AppApi/Include
    ${SDK_COMPONENTS_BASE_DIR}/MAC/Include
    ${SDK_COMPONENTS_BASE_DIR}/MiniMac/Include
    ${SDK_COMPONENTS_BASE_DIR}/MMac/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Include
    ${SDK_COMPONENTS_BASE_DIR}/MicroSpecific/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZPSAPL/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZPSNWK/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZPSMAC/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZPSTSV/Include
    ${SDK_COMPONENTS_BASE_DIR}/PDUM/Include
    ${SDK_COMPONENTS_BASE_DIR}/PDM/Include
    ${SDK_COMPONENTS_BASE_DIR}/PWRM/Include
    ${SDK_COMPONENTS_BASE_DIR}/HardwareAPI/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Include
    ${SDK_COMPONENTS_BASE_DIR}/TimerServer/Include
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Include
    ${SDK_COMPONENTS_BASE_DIR}/Random/Include
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Include

    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/Common
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/NwkSteering
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SDK_INCLUDE_FOLDERS}
    ${PROJECT_SOURCE_DIR}/firmwares
    ${PROJECT_SOURCE_DIR}/firmwares/common/include
    ${PROJECT_SOURCE_DIR}/firmwares/${APP_TARGET}
)

link_directories(
    ${SDK_COMPONENTS_BASE_DIR}/Library
    ${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build/AppBuildZBPro.ld")

set(ZPS_CONFIG "${SDK_PREFIX}/Tools/ZPSConfig/ZPSConfig.py")
set(PDUM_CONFIG "${SDK_PREFIX}/Tools/PDUMConfig/PDUMConfig.py")
set(ZPSCFG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${APP_TARGET}/app.zpscfg")
add_custom_command(
    OUTPUT
        zps_gen.c
        zps_gen.h
    COMMAND uv run --directory ${VENV_DIR}
             ${ZPS_CONFIG}
            -n ${APP_TARGET}
            -f ${ZPSCFG_FILE}
            -o ${CMAKE_CURRENT_BINARY_DIR}
            -t ${JENNIC_CHIP}
            -l ${SDK_PREFIX}/Components/Library/libZPSNWK_${JENNIC_CHIP_FAMILY}.a
            -a ${SDK_PREFIX}/Components/Library/libZPSAPL_${JENNIC_CHIP_FAMILY}.a
            -c ${TOOLCHAIN_ABS_PATH}
)
add_custom_command(
    OUTPUT
        pdum_gen.c
        pdum_gen.h
        pdum_apdu.S
    COMMAND uv run --directory ${VENV_DIR}
            ${PDUM_CONFIG}
            -z ${APP_TARGET}
            -f ${ZPSCFG_FILE}
            -o ${CMAKE_CURRENT_BINARY_DIR}
)

# ============= START - SDK library =============
set(SDK_SRC
    pdum_gen.c
    zps_gen.c

    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/app_zps_link_keys.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/appZpsBeaconHandler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/appZdpExtraction.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZQueue.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/ZTimer.c
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/portasm_${JENNIC_CHIP_FAMILY}.S
    ${SDK_COMPONENTS_BASE_DIR}/ZigbeeCommon/Source/port_${JENNIC_CHIP_FAMILY}.c

    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/dlist.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_attribute.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_buffer.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_command.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_common.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_configureReportingCommandHandle.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_CustomCommandReceive.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_CustomCommandSend.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_defaultResponse.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_event.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_heap.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_library_options.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_PDUbufferReadWrite.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_PDUbufferReadWriteString.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_readAttributesRequestHandle.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_readAttributesResponseHandle.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_WriteAttributesRequestHandle.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_reportManager.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_reportMaths.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_reportScheduler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_reportStringHandling.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_reportStructure.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_search.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_timer.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCIF/Source/zcl_transmit.c

    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/Basic.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/BasicCommandHandler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/Groups.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/GroupsCommandHandler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/Identify.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/IdentifyCommandHandler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/IdentifyServerCommands.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/OnOff.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/OnOffCommands.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/OnOffCommandHandler.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/MultiStateInputBasic.c
    ${SDK_COMPONENTS_BASE_DIR}/ZCL/Clusters/General/Source/PowerConfiguration.c

    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/Common/bdb_fr.c
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/Common/bdb_start.c
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/Common/bdb_state_machine.c
    ${SDK_COMPONENTS_BASE_DIR}/BDB/Source/NwkSteering/bdb_ns.c
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
    ZPSMAC_Mini_SOC_${JENNIC_CHIP_FAMILY}
    PWRM_${JENNIC_CHIP_FAMILY}
    ZPSTSV_${JENNIC_CHIP_FAMILY}
    AES_SW_${JENNIC_CHIP_FAMILY}
    PDUM_${JENNIC_CHIP_FAMILY}
    ZPSAPL_${JENNIC_CHIP_FAMILY}
    ZPSAPL_ZCP-GU_${JENNIC_CHIP_FAMILY}
    Random_${JENNIC_CHIP_FAMILY}
    PDM_EEPROM_NO_RTOS_${JENNIC_CHIP_FAMILY}
    ZPSNWK_ZED_${JENNIC_CHIP_FAMILY}
    DBG_${JENNIC_CHIP_FAMILY}
)
# ============= END - SDK library =============

# ============= START - Firmware =============
set(APP_SRC
    common/src/irq_JN516x.S

    common/src/app_basic_endpoint.c
    common/src/app_battery.c
    common/src/app_buttons.c
    common/src/app_events.c
    common/src/app_init.c
    common/src/app_led.c
    common/src/app_main.c
    common/src/app_node.c
    common/src/app_on_off_endpoint.c
    common/src/app_polling.c
    common/src/app_reporting.c
    common/src/app_resources.c
    common/src/app_zcl_tick.c
    common/src/ConfigurationCluster.c
    common/src/isr_system.c
    ${APP_TARGET}/device_config.c

    zps_gen.c
    pdum_gen.c
    pdum_apdu.S
)
add_executable(${APP_TARGET} ${APP_SRC})

target_compile_definitions(${APP_TARGET} PRIVATE
    DEBUG_BASIC_EP
    DEBUG_BATTERY
    DEBUG_BUTTON
    DEBUG_EVENTS
    DEBUG_INIT
    DEBUG_LED
    DEBUG_MAIN
    DEBUG_NODE
    DEBUG_ON_OFF_EP
    DEBUG_POLL
    DEBUG_RESOURCES
    DEBUG_REPORT
    DEBUG_ZCL_TICK
    DEBUG_CONFIGURATION_CLUSTER
    DEBUG_ISR_SYSTEM
)

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
