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

#pragma once

#include <shm_common.h>

#include <cstdint>
#include <string>
#include <vector>
namespace tal
{

struct TelemetryData
{
    std::string devicePath; // Dbus object path of telemetry object.
    std::string interface;  // Phosphor D-Bus interface of telemetry object.
    std::string propName;   // Metric name.
    std::vector<uint8_t> rawData; // Raw data of the value.
    int rc;                       // Set this value to non zero for nan update.
    nv::sensor_aggregation::DbusVariantType value; // Metric value.
    std::string associatedEntityPath; // Optional for other metrics. Required
                                      // for platform environment metrics.
    uint64_t timestamp;               // Timestamp of the telemetry update.
};
} // namespace tal
