/**********************************************************************
 * Copyright (C) 2015-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          Do not edit it manually.
 **********************************************************************/
#ifndef HPNFC_OBJ_IF_H
#define HPNFC_OBJ_IF_H

#include "hpnfc_if.h"

/** @defgroup DriverObject Driver API Object
 *  API listing for the driver. The API is contained in the object as
 *  function pointers in the object structure. As the actual functions
 *  resides in the Driver Object, the client software must first use the
 *  global GetInstance function to obtain the Driver Object Pointer.
 *  The actual APIs then can be invoked using obj->(api_name)() syntax.
 *  These functions are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * API methods
 **********************************************************************/
typedef struct HPNFC_OBJ_s
{
    /**
     * "Get the driver's memory requirements to support the configuration
     * of the given controller."
     * @param[in] registerBase base address of controller registers
     * @param[out] req on success, filled with the number of bytes the driver requires for private data.
     * @return 0 on success (dataSize filled)
     * @return ENOTSUP if configuration cannot be supported due to driver/hardware constraints
     */
    uint32_t (*probe)(uintptr_t registerBase, HPNFC_SysReq* req); // HPNFC_Probe

    /**
     * The init function makes the following jobs: - initializes the
     * driver instance and state - configures the hardware as specified
     * by the client - initializes hardware
     * @param[in] privateData pointer to memory allocated for the driver use of sufficient size to hold the number of bytes returned by probe in HPNFC_SysReq
     * @param[in] config contains driver and hardware configuration as determined by probe call(s)
     * @return 0 on success
     * @return EINVAL if config is NULL or if illegal/inconsistent values in config
     */
    int32_t (*init)(void* privateData, HPNFC_Config* config); // HPNFC_Init

    /**
     * "Driver ISR.  Platform-specific code is responsible for ensuring
     * this gets called when the controller's interrupt is asserted.
     * Registering the ISR should be done after calling init, and before
     * calling start."
     * @param[in] privateData driver instance data filled by init
     */
    void (*isr)(void* privateData); // HPNFC_Isr

    /**
     * "Start the driver. This enables any interrupts handled by the
     * driver. This is called after the client has successfully
     * initialized the driver and hooked the driver's ISR (the isr member
     * of this struct) to the IRQ."
     * @param[in] privateData driver instance data filled by init
     */
    void (*start)(void* privateData);

    /**
     * Stop the driver. This disables any interrupts being handled by the
     * driver.
     * @param[in] privateData driver instance data filled by init
     */
    void (*stop)(void* privateData);

    /**
     * Destroy the driver (automatically performs a stop)
     * @param[in] privateData driver instance data filled by init
     */
    void (*destroy)(void* privateData);

    /**
     * Function assembles CDMA descriptor base on cdmaCmd
     * @param[in] privateData driver instance data filled by init
     * @param[in] descPtr descriptor pointer
     * @param[in] cdmaCmd CDMA command used to assemble one CDMA descriptor
     * @return 0 on success
     * @return EINVAL if descPtr is NULL
     */
    int32_t (*assembleCdmaDesc)(void* privateData, void* descPtr, HPNFC_CdmaCmd* cdmaCmd); // HPNFC_AssembleCdmaDesc

    /**
     * Function executes request
     * @param[in] privateData driver instance data filled by init
     * @param[in] cmdReq Request to execute
     * @return 0 on success
     * @return EINVAL if cmdReq is NULL or if illegal/inconsistent values in cmdReq
     * @return EBUSY if given thread defined in cmdReq is busy
     */
    int32_t (*execCmdReq)(void* privateData, HPNFC_CmdReq* cmdReq); // HPNFC_ExecCmdReq

    /**
     * Function change controller and device working mode.
     * @param[in] privateData driver instance data filled by init
     * @param[in] workMode work mode to set up
     * @param[in] timingMode timing mode of work mode
     * @param[in] phyConfig phy configuration for given work mode
     * @return 0 on success
     * @return EINVAL if phyConfig is NULL
     * @return EBUSY if command send to memory device to change its mode cannot be finished
     */
    int32_t (*setWorkMode)(void* privateData, HPNFC_WorkMode workMode, uint8_t timingMode, HPNFC_PhyConfig* phyConfig);

    /**
     * Function configures ECC module of controller
     * @param[in] privateData driver instance data filled by init
     * @param[in] eccConfig new ECC configuration
     * @return 0 on success
     * @return EINVAL if eccConfig is NULL or if illegal/inconsistent values in eccConfig
     * @return ENOTSUP if ECC feature is not supported by controller
     * @return EBUSY if controller is busy
     */
    int32_t (*setEccConfig)(void* privateData, HPNFC_EccConfiguration* eccConfig);

    /**
     * Function get current ECC configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] eccConfig current ECC configuration
     * @return 0 on success
     * @return EINVAL if eccConfig is NULL
     * @return ENOTSUP if ECC feature is not supported by controller
     */
    int32_t (*getEccConfig)(void* privateData, HPNFC_EccConfiguration* eccConfig);

    /**
     * Function sets transfer configuration for program and read page
     * operations
     * @param[in] privateData driver instance data filled by init
     * @param[in] transferCfg new transfer configuration configuration
     * @return 0 on success
     * @return EINVAL if transferCfg is NULL or if illegal/inconsistent values in transferCfg
     * @return EBUSY if controller is busy
     */
    int32_t (*setTransferConfig)(void* privateData, HPNFC_TransferConfig* transferCfg);

    /**
     * Function gets transfer configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] transferCfg current transfer configuration configuration
     * @return 0 on success
     * @return EINVAL if transferCfg is NULL
     * @return EBUSY if controller is busy
     */
    int32_t (*getTransferConfig)(void* privateData, HPNFC_TransferConfig* transferCfg);

    /**
     * Function can be used to get first failed descritor from chain of
     * descriptors
     * @param[in] cdmaDesc first descriptor in the chain
     * @param[out] fail a pointer to the descriptor whose execution failed
     * @return 0 on success
     * @return EINVAL if *fail is NULL
     */
    int32_t (*cdmaGetFailedDesc)(void* cdmaDesc, void** fail);

    /**
     * Function can be used to get next failed descriptor. Current
     * descriptor is not checked. The search begins from the next
     * descriptor.
     * @param[in] cdmaDesc a descriptor in the chain
     * @param[out] failed a pointer to the descriptor whose execution failed
     * @return 0 on success
     * @return EINVAL if *failed is NULL
     */
    int32_t (*cdmaGetNextFailedDesc)(void* cdmaDesc, void** failed);

    /**
     * The function returns the detailed status of the CDMA operation
     * @param[in] privateData driver instance data filled by init
     * @param[in] cdmaDesc a descriptor to check
     * @param[out] operationResult status of operation defined in descriptor
     * @return 0 on success
     * @return EINVAL if operationResult is NULL
     */
    int32_t (*cdmaDescGetOperationResult)(void* privateData, void* cdmaDesc, HPNFC_OperationResult* operationResult);

    /**
     * The function returns the detailed status of a operation from HPNFC
     * registers It can be used when the HPNFC controller is not able to
     * update status inside a descriptor because of data integrity error.
     * Then status can be read from registers instead of reading it from
     * a descriptor.
     * @param[in] privateData driver instance data filled by init
     * @param[in] cmdReq a request to check
     * @param[out] operationResult status of operation defined in descriptor
     * @return 0 on success
     * @return EINVAL if operationResult is NULL
     */
    int32_t (*reqGetOperationResult)(void* privateData, HPNFC_CmdReq* cmdReq, HPNFC_OperationResult* operationResult);

    /**
     * Function create cdmaCmd command base on descriptor.
     * @param[in] privateData driver instance data filled by init
     * @param[in] descPtr descriptor pointer to parse
     * @param[out] cdmaCmd CDMA command created base on descriptor pointer
     * @return 0 on success
     * @return EINVAL if cdmaCmd is NULL
     */
    int32_t (*cdmaParseDesc)(void* privateData, void* descPtr, HPNFC_CdmaCmd* cdmaCmd);  // HPNFC_CdmaParseDesc

    /**
     * Function sets status polling configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] statusPollingConfig new status polling configuration
     * @return 0 on success
     * @return EINVAL if statusPollingConfig is NULL
     * @return EBUSY if controller is busy
     */
    int32_t (*setStatusPollingConfig)(void* privateData, HPNFC_StatusPollingConfig* statusPollingConfig);

    /**
     * Function gets status polling configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] statusPollingConfig current status polling configuration
     * @return 0 on success
     * @return EINVAL if statusPollingConfig is NULL
     */
    int32_t (*getStatusPollingConfig)(void* privateData, HPNFC_StatusPollingConfig* statusPollingConfig);

    /**
     * Function sets auto status configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] autoStatusConfig new status configuration
     * @return 0 on success
     * @return EINVAL if autoStatusConfig is NULL
     * @return EBUSY if controller is busy
     */
    int32_t (*setAutoStatusConfig)(void* privateData, HPNFC_AutoStatusConfig* autoStatusConfig);

    /**
     * Function gets auto status configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] autoStatusConfig current auto status configuration
     * @return 0 on success
     * @return EINVAL if autoStatusConfig is NULL
     */
    int32_t (*getAutoStatusConfig)(void* privateData, HPNFC_AutoStatusConfig* autoStatusConfig);

    /**
     * Function gets device layout
     * @param[in] privateData driver instance data filled by init
     * @param[out] deviceLayout current device layout
     * @return 0 on success
     * @return EINVAL if deviceLayout is NULL
     */
    int32_t (*getDeviceLayout)(void* privateData, HPNFC_DeviceLayout* deviceLayout); // HPNFC_GetDeviceLayout

    /**
     * Function sets device sublayout
     * @param[in] privateData driver instance data filled by init
     * @param[in] deviceSubLayout new device sublayout to set
     * @return 0 on success
     * @return EINVAL if deviceSubLayout is NULL
     * @return EBUSY if controller is busy
     */
    int32_t (*setDeviceSubLayout)(void* privateData, HPNFC_DeviceSubLayout* deviceSubLayout);

    /**
     * Function sets LUN interleaved configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] lunInterleavedConfig new interleaved configuration to set
     * @return 0 on success
     * @return EINVAL if lunInterleavedConfig is NULL if illegal/inconsistent values in lunInterleavedConfig
     * @return EBUSY if controller is busy
     */
    int32_t (*setLunInterleavedConfig)(void* privateData, HPNFC_LunInterleavedConfig* lunInterleavedConfig);

    /**
     * Function gets LUN interleaved configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] lunInterleavedConfig current interleaved configuration
     * @return 0 on success
     * @return EINVAL if lunInterleavedConfig is NULL
     */
    int32_t (*getLunInterleavedConfig)(void* privateData, HPNFC_LunInterleavedConfig* lunInterleavedConfig);

    /**
     * Function gets current Ready/Busy# line status of one chip
     * @param[in] privateData driver instance data filled by init
     * @param[in] chip chip number to check rnb signal
     * @param[out] rnb current chip Ready/Busy# signal state
     * @return 0 on success
     * @return EINVAL if rnb is NULL
     */
    int32_t (*getRnb)(void* privateData, uint8_t chip, uint8_t* rnb); // HPNFC_GetRnb

    /**
     * Function gets status of boot operation
     * @param[in] privateData driver instance data filled by init
     * @param[out] bootStatus boot status
     * @param[out] cpyId boot block used in the latest boot sequence run
     * @return 0 on success
     * @return EINVAL if cpyId is NULL
     * @return ENOTSUP if boot feature is not supported by controller
     */
    int32_t (*getBootStatus)(void* privateData, HPNFC_BootStatus* bootStatus, uint8_t* cpyId);

    /**
     * Function gets multiplane configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] multiplaneConfig current multiplane configuration
     * @return 0 on success
     * @return EINVAL if multiplaneConfig is NULL
     */
    int32_t (*getMultiplaneConfig)(void* privateData, HPNFC_MultiplaneConfig* multiplaneConfig);

    /**
     * Function sets multiplane operations configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] multiplaneConfig new multiplane configuration to set
     * @return 0 on success
     * @return EINVAL if multiplaneConfig is NULL if illegal/inconsistent values in multiplaneConfig
     * @return EBUSY if controller is busy
     */
    int32_t (*setMultiplaneConfig)(void* privateData, HPNFC_MultiplaneConfig* multiplaneConfig);

    /**
     * Function gets cache operations configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] cacheConfig current cache configuration
     * @return 0 on success
     * @return EINVAL if cacheConfig is NULL
     */
    int32_t (*getCacheConfig)(void* privateData, HPNFC_CacheConfig* cacheConfig);

    /**
     * Function sets cache operations configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] cacheConfig new cache configuration
     * @return 0 on success
     * @return EINVAL if cacheConfig is NULL
     * @return EBUSY if controller is busy
     */
    int32_t (*setCacheConfig)(void* privateData, HPNFC_CacheConfig* cacheConfig);

    /**
     * Function gets ECC BCH configuration info (supported features)
     * @param[in] privateData driver instance data filled by init
     * @param[out] bchConfigInfo BCH supported configurations
     * @return 0 on success
     * @return EINVAL if bchConfigInfo is NULL
     */
    int32_t (*getBchConfigInfo)(void* privateData, HPNFC_BchConfigInfo* bchConfigInfo);

    /**
     * Function gets controller info (supported features)
     * @param[in] privateData driver instance data filled by init
     * @param[out] controllerInfo Controller supported features
     * @return 0 on success
     * @return EINVAL if controllerInfo is NULL
     */
    int32_t (*getControllerInfo)(void* privateData, HPNFC_ControllerInfo* controllerInfo); // HPNFC_GetControllerInfo

    /**
     * Function gets CDMA descriptor size in bytes which is expected by
     * hardware.
     * @param[in] privateData driver instance data filled by init
     * @param[out] descSize descriptor size in bytes
     * @return 0 on success
     * @return EINVAL if descSize is NULL
     */
    int32_t (*getCdmaDescSize)(void* privateData, uint8_t* descSize);

    /**
     * Function gets debug data using HPNFC Debug Interface.
     * @param[in] privateData driver instance data filled by init
     * @param[inout] dbgReq debug request to be executed on Debug Interface
     * @return 0 on success
     * @return EINVAL if dbgReq is NULL or if illegal/inconsistent values in dbgReq
     * @return EIO if register debug interface is not selected
     * @return ENOTSUP if HPNFC controller does not support debug feature
     */
    int32_t (*getDebugData)(void* privateData, HPNFC_DebugReq* dbgReq);

    /**
     * Function sets Debug Interface type
     * @param[in] privateData driver instance data filled by init
     * @param[in] ifType debug nterface type
     * @return 0 on success
     * @return EINVAL if ifType has wrong value
     * @return ENOTSUP if HPNFC controller does not support debug feature
     */
    int32_t (*setDebugIfType)(void* privateData, HPNFC_DebugIfType ifType);

    /**
     * Function gets Debug Interface type
     * @param[in] privateData driver instance data filled by init
     * @param[out] ifType debug interface type
     * @return 0 on success
     * @return EINVAL if ifType has wrong value
     * @return ENOTSUP if HPNFC controller does not support debug feature
     */
    int32_t (*getDebugIfType)(void* privateData, HPNFC_DebugIfType* ifType);

    /**
     * Function sets data integrity configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] diConfig data integrity configuration
     * @return 0 on success
     * @return EINVAL if diConfig is NULL or if illegal/inconsistent values in diConfig
     * @return ENOTSUP if HPNFC controller does not support data integrity feature
     * @return EBUSY if controller is busy
     */
    int32_t (*setDiConfig)(void* privateData, HPNFC_DiConfig* diConfig); // HPNFC_SetDiConfig

    /**
     * Function current data integrity configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] diConfig data integrity configuration
     * @return 0 on success
     * @return EINVAL if privateData  or diConfig is NULL
     * @return ENOTSUP if HPNFC controller does not support data integrity feature
     */
    int32_t (*getDiConfig)(void* privateData, HPNFC_DiConfig* diConfig);

    /**
     * Function sets error injection configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] diErrInjConfig error injection configuration to set
     * @return 0 on success
     * @return EINVAL if diErrInjConfig is NULL or if illegal/inconsistent values in diErrInjConfig
     * @return ENOTSUP if HPNFC controller does not support data integrity feature
     * @return EBUSY if controller is busy
     */
    int32_t (*setDiErrInjConfig)(void* privateData, HPNFC_DiErrInjConfig* diErrInjConfig); // HPNFC_SetDiErrInjConfig

    /**
     * Function gets error injection configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] diErrInjConfig read error injection configuration
     * @return 0 on success
     * @return EINVAL if diErrInjConfig is NULL or if illegal/inconsistent values in diErrInjConfig
     * @return ENOTSUP if HPNFC controller does not support data integrity feature
     */
    int32_t (*getDiErrInjConfig)(void* privateData, HPNFC_DiErrInjConfig* diErrInjConfig);

    /**
     * Function sets DMA configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] dmaConfig DMA configuration
     * @return 0 on success
     * @return EINVAL if dmaConfig is NULL
     * @return ENOTSUP if HPNFC controller does not support configuration of DMA
     */
    int32_t (*setDmaConfig)(void* privateData, HPNFC_DmaConfig* dmaConfig);

    /**
     * Function gets DMA configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] dmaConfig DMA configuration
     * @return 0 on success
     * @return EINVAL if dmaConfig is NULL
     * @return ENOTSUP if HPNFC controller does not support configuration of DMA
     */
    int32_t (*getDmaConfig)(void* privateData, HPNFC_DmaConfig* dmaConfig);

    /**
     * Function writes single record of remap table
     * @param[in] privateData driver instance data filled by init
     * @param[in] remapRecord remap record to write
     * @param[in determines which memory target is bind to current write] memTrg
     * @return 0 on success
     * @return EINVAL if remapRecord is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*writeRemapRecord)(void* privateData, HPNFC_RemapRecord* remapRecord, uint8_t memTrg); // HPNFC_WriteRemapRecord

    /**
     * Function reads single record of remap table
     * @param[in] privateData driver instance data filled by init
     * @param[in] recordIdx index of remap record
     * @param[out] remapRecord read remap record
     * @param[in determines which memory target is bind to current read] memTrg
     * @return 0 on success
     * @return EINVAL if remapRecord is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*readRemapRecord)(void* privateData, uint16_t recordIdx, HPNFC_RemapRecord* remapRecord, uint8_t memTrg);

    /**
     * Function clears remap table
     * @param[in] privateData driver instance data filled by init
     * @return 0 on success
     * @return EINVAL if privateData is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*clearRemapTable)(void* privateData);

    /**
     * Function gets number of records stored in remap table
     * @param[in] privateData driver instance data filled by init
     * @param[out] recordCount number of records stored in remap table
     * @return 0 on success
     * @return EINVAL if recordCount is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*getRemapRecordCount)(void* privateData, uint16_t* recordCount);

    /**
     * Function sets remap mask
     * @param[in] privateData driver instance data filled by init
     * @param[in] remapMask remap mask to be set
     * @return 0 on success
     * @return EINVAL if privateData is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*setRemapMask)(void* privateData, uint32_t remapMask);

    /**
     * Function gets remap mask
     * @param[in] privateData driver instance data filled by init
     * @param[out] remapMask remap mask to be set
     * @return 0 on success
     * @return EINVAL if remapMask is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*getRemapMask)(void* privateData, uint32_t* remapMask);

    /**
     * Function sets remap enable
     * @param[in] privateData driver instance data filled by init
     * @param[in] remapEnable configuration of remap, 0 - disabled, 1 - enabled
     * @return 0 on success
     * @return EINVAL if privateData is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*setRemapEnable)(void* privateData, uint8_t remapEnable); // HPNFC_SetRemapEnable

    /**
     * Function gets remap enable
     * @param[in] privateData driver instance data filled by init
     * @param[out] remapEnable configuration of remap, 0 - disabled, 1 - enabled
     * @return 0 on success
     * @return EINVAL if remapEnable is NULL
     * @return ENOTSUP if HPNFC controller does not support remap mechanism
     */
    int32_t (*getRemapEnable)(void* privateData, uint8_t* remapEnable);

    /**
     * Function sets pSLC configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] pSlcConfig configuration pSLC to set
     * @return 0 on success
     * @return EINVAL if pSlcConfig.pSlcPrefixEn > 1
     * @return ENOTSUP if HPNFC controller does not support pSLC feautre
     */
    int32_t (*setPSlcConfig)(void* privateData, HPNFC_PSlcConfig* pSlcConfig);

    /**
     * Function gets pSLC configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] pSlcConfig current pSlc configuration
     * @return 0 on success
     * @return EINVAL if privateData is NULL
     * @return ENOTSUP if HPNFC controller does not support pSLC feautre
     */
    int32_t (*getPSlcConfig)(void* privateData, HPNFC_PSlcConfig* pSlcConfig);

    /**
     * Function sets ZQ calbration configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in] zqCalConfig configuration of ZQ calibration to set
     * @return 0 on success
     * @return EINVAL if zqCalConfig is NULL or if illegal/inconsistent values in zqCalConfig
     */
    int32_t (*setZqCalConfig)(void* privateData, HPNFC_ZqCalConfig* zqCalConfig);

    /**
     * Function gets ZQ calbration configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out] zqCalConfig current configuration of ZQ calibration
     * @return 0 on success
     * @return EINVAL if zqCalConfig is NULL
     */
    int32_t (*getZqCalConfig)(void* privateData, HPNFC_ZqCalConfig* zqCalConfig);

    /**
     * Function sets ZQ calbration enable
     * @param[in] privateData driver instance data filled by init
     * @param[in] target target number - bank or volume, depending on HPNFC configuration
     * @param[in] enable parameter is used to enable or disable ZQ calibration functionality
     *    1 - enable, 0 - disable
     * @return 0 on success
     * @return EINVAL if enable is bigger than 1
     */
    int32_t (*setZqCalEnable)(void* privateData, uint8_t target, uint8_t enable);

    /**
     * Function gets ZQ calbration enable
     * @param[in] privateData driver instance data filled by init
     * @param[in] target target number - bank or volume, depending on HPNFC configuration
     * @param[out] enable iforms whether ZQ calibration is enabled or disabled
     *    1 - enabled, 0 - disabled
     * @return 0 on success
     * @return EINVAL if target is bigger than 15
     */
    int32_t (*getZqCalEnable)(void* privateData, uint8_t target, uint8_t* enable);

    /**
     * Function gets ZQ calbration stat
     * @param[in] privateData driver instance data filled by init
     * @param[out] calStat Each bit of calStat informs about ZQ recalibration status for
     *      selected target/device Bits are encoded as following
     *      b0 - No error was detected during re-calibration process
     *      b1 - Error was detected during re-calibration process
     * @return 0 on success
     * @return EINVAL if calStat is NULL
     */
    int32_t (*getZqStat)(void* privateData, uint16_t* calStat);

    /**
     * Function clears ZQ calbration stat
     * @param[in] privateData driver instance data filled by init
     * @param[int] calStatToClear Each bit of calStat points ZQ recalibration status to clear for
     *      selected target/device Bits are encoded as following
     *      b0 - do not clear error
     *      b1 - clear re-calibration error
     * @return 0 on success
     * @return EINVAL if calStatToClear is NULL
     */
    int32_t (*clearZqStat)(void* privateData, uint16_t calStatToClear);

    /**
     * Function sets timeout configuration
     * @param[in] privateData driver instance data filled by init
     * @param[in timeout configuration to set] timeoutConfig
     * @return 0 on success
     * @return EINVAL if timeoutConfig is NULL
     */
    int32_t (*setTimeoutConfig)(void* privateData, HPNFC_TimeoutConfig* timeoutConfig);

    /**
     * Function gets timeout configuration
     * @param[in] privateData driver instance data filled by init
     * @param[out current timeout configuration] timeoutConfig
     * @return 0 on success
     * @return EINVAL if timeoutConfig is NULL
     */
    int32_t (*getTimeoutConfig)(void* privateData, HPNFC_TimeoutConfig* timeoutConfig);

    /**
     * Function sets CE pin reduction
     * @param[in] privateData driver instance data filled by init
     * @param[in configure CE pin reduction 0 - disable CE pin reduction 1 - enable CE pin reduction] enable
     * @return 0 on success
     * @return EINVAL if enable is bigger than 1
     */
    int32_t (*setCePinReduction)(void* privateData, uint8_t enable);

    /**
     * Function sets CE pin reduction
     * @param[in] privateData driver instance data filled by init
     * @param[out conifgure CE pin reduction] enabled
     * @return 0 on success
     * @return EINVAL if privateData is NULL
     */
    int32_t (*getCePinReduction)(void* privateData, uint8_t* enabled);

    /**
     * Function gets information about NAND flash device
     * @param[in] privateData driver instance data filled by init
     * @param[out] deviceInfo current device layout
     * @return 0 on success
     * @return EINVAL if deviceInfo is NULL
     */
    int32_t (*getDeviceInfo)(void* privateData, HPNFC_DevInfo* deviceInfo);

} HPNFC_OBJ;

/**
 * In order to access the HPNFC APIs, the upper layer software must call
 * this global function to obtain the pointer to the driver object.
 * @return HPNFC_OBJ* Driver Object Pointer
 */
extern HPNFC_OBJ *HPNFC_GetInstance(void);

/**
 *  @}
 */


#endif	/* HPNFC_OBJ_IF_H */
