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

#include "tal_shmem.hpp"

#include <telemetry_mrd_client.hpp>
#include <telemetry_mrd_producer.hpp>

int tal::SharedMemModule::updateTelemetry(
    const std::string& devicePath, const std::string& interface,
    const std::string& propName, std::vector<uint8_t>& rawData,
    const uint64_t timestamp, int rc,
    nv::sensor_aggregation::DbusVariantType& value,
    const std::string associatedEntityPath)
{
    return nv::shmem::AggregationService::updateTelemetry(
        devicePath, interface, propName, value, timestamp, rc,
        associatedEntityPath);
}

bool tal::SharedMemModule::namespaceInit(const std::string& processName)
{
    lg2::info(
        "Initializing shmem namespaceInit with process name:{PROCESSNAME}",
        "PROCESSNAME", processName);
    return nv::shmem::AggregationService::namespaceInit(processName);
}

std::vector<nv::shmem::SensorValue>
    tal::SharedMemModule::getAllMrds(const std::string& mrdNamespace)
{
    return nv::shmem::sensor_aggregation::getAllMRDValues(mrdNamespace);
}

std::vector<std::string> tal::SharedMemModule::getMrdNamespaces()
{
    return nv::shmem::sensor_aggregation::getMrdNamespacesValues();
}
