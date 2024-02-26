/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <shm_common.h>
#include <phosphor-logging/lg2.hpp>

namespace tal {
enum class TalModule {
  Smbus,
  SharedMem,
};


class TelemetryHandler {

  public:
    // Method to initialize namespace specific operations
    /**
     * @brief Interface to initialize the namespace for module.
     *
     * @param[in] processName - process name of producer service
     * @return - bool value true if namespaceinit is success.
     */
    virtual bool namespaceInit(const std::string& processName) = 0;

    // Method to update telemetry
    /**
     * @brief interface to call update telemetry for each of the telemetry module
     *
     * @param[in] devicePath - Device path of telemetry object.
     * @param[in] interface - Phosphor D-Bus interface of telemetry object.
     * @param[in] propName - Metric name.
     * @param[in] rawData - Raw data for the smbus interface.
     * @param[in] timestamp - Timestamp of telemetry object.
     * @param[in] rc - Set this value to non zero for nan update.
     * @param[in] value - Metric value.
     * @param[in] associatedEntityPath - optional for other metrics. Required for
     * platform environment metrics.
     */
    virtual int updateTelemetry(const std::string& devicePath,
                                const std::string& interface,
                                const std::string& propName,
                                std::vector<uint8_t>& rawData,
                                const uint64_t timestamp, int rc,
                                nv::sensor_aggregation::DbusVariantType& value,
                                const std::string associatedEntityPath = {}) = 0;

    // Method to get all MRDs
    /**
     * @brief This Interface gives all metric report.
     *
     * @param[in] mrdNamespace - metric report definitions namespace
     * @return values - metric report definitions values. Incase of no elements or
     * absence of given namespace exception is thrown.
     */
    virtual std::vector<nv::shmem::SensorValue> getAllMrds(const std::string& mrdNamespace) = 0;

    virtual ~TelemetryHandler() = default;
};
}
