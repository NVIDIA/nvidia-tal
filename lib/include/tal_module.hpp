/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
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

    // Method to get all metric repor collection from the shmem
    /**
     * @brief This API returns all metric report collection for a shared mem namespace 
     *
     * @return values - metric report collection values.
     */
    virtual std::vector<std::string> getMrdNamespaces() {return {};};

    virtual ~TelemetryHandler() = default;
};
}
