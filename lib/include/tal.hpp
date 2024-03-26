/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/*
TAL will use the APIs defined in this header to update, intialise and get the
telemetry objects. APIs provided in this header will be used for

- Initialize and create the telemetry module
- Update Value and/OR Timestamp values
- get Value and/OR Timestamp values

Below are some examples on API usage, refer API documentation section in this
file for more details.

Init namespace:
*******************************************************************************
    This API is to initialize the each module specific namespace. This API takes
	the process name as input and enumeration of process type client or producer.
	This API should also invokes the tal init which intializes the telemetry 
	modules  .

Update telemetry:
*******************************************************************************
    API to call update telemetry for each of the telemetry module. It will fan 
	out update telemetry call to each telemetry module.

Get all MRDs:
*******************************************************************************
	This API returns all metric report definitions for a shared mem namespace 
	others are skipped. This API should be used by MRD clients such as bmcweb. 
*/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <shm_common.h>

namespace tal {
enum class ProcessType {
  Producer,
  Client
};

class TelemetryAggregator {
  public:

    // Function to initialize all modules
    /**
     * @brief API to initialize the namespace for each module and initialize tal modules.
     *
     * @param[in] processName - process name of producer service
     * @param[in] ProcessType type - process type can be client or producer.
     * @return - gives false if any of the namespaceinit fails.
     */
    static bool namespaceInit(ProcessType type, const std::string& processName = {});


    // Method to update telemetry for all modules
    /**
     * @brief API to call update telemetry for each of the telemetry module
     *
     * @param[in] devicePath - Device path of telemetry object.
     * @param[in] interface - Phosphor D-Bus interface of telemetry object.
     * @param[in] propName - Metric name.
     * @param[in] rawData - raw data of the value.
     * @param[in] value - Metric value.
     * @param[in] timestamp - Timestamp of telemetry object.
     * @param[in] rc - Set this value to non zero for nan update.
     * @param[in] associatedEntityPath - optional for other metrics. Required for
     * platform environment metrics.
     */
    static void updateTelemetry(const std::string& devicePath,
                                const std::string& interface,
                                const std::string& propName,
                                std::vector<uint8_t>& rawData,
                                const uint64_t timestamp, int rc,
                                nv::sensor_aggregation::DbusVariantType& value,
                                const std::string associatedEntityPath = {});

    // Method to get all MRDs
    /**
     * @brief This API returns all metric report definitions for a shared mem namespace 
     * others are skipped
     *
     * @param[in] mrdNamespace - metric report definitions namespace
     * @return values - metric report definitions values. Incase of no elements or
     * absence of given shared memory namespace exception is thrown.
     */
    static std::vector<nv::shmem::SensorValue> getAllMrds(const std::string& mrdNamespace);
};
} // tal namespace
