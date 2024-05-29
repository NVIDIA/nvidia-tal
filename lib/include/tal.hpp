/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION &
 * AFFILIATES. All rights reserved. SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    the process name as input and enumeration of process type client or
producer. This API should also invokes the tal init which intializes the
telemetry modules  .

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

#include <shm_common.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace tal
{
enum class ProcessType
{
    Producer,
    Client
};

class TelemetryAggregator
{
  public:
    // Function to initialize all modules
    /**
     * @brief API to initialize the namespace for each module and initialize tal
     * modules.
     *
     * @param[in] processName - process name of producer service
     * @param[in] ProcessType type - process type can be client or producer.
     * @return - gives false if any of the namespaceinit fails.
     */
    static bool namespaceInit(ProcessType type,
                              const std::string& processName = {});

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
     * @param[in] associatedEntityPath - optional for other metrics. Required
     * for platform environment metrics.
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
     * @brief This API returns all metric report definitions for a shared mem
     * namespace others are skipped
     *
     * @param[in] mrdNamespace - metric report definitions namespace
     * @return values - metric report definitions values. Incase of no elements
     * or absence of given shared memory namespace exception is thrown.
     */
    static std::vector<nv::shmem::SensorValue>
        getAllMrds(const std::string& mrdNamespace);

    // Method to get all metric report collection from the shmem
    /**
     * @brief This API returns all metric report collection for a shared mem
     * namespace
     *
     * @return values - metric report collection values.
     */
    static std::vector<std::string> getMrdNamespaces();
};
} // namespace tal
