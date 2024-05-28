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

#include "tal_smbus.hpp"

#ifdef ENABLE_SMBUS
#include "smbus_telemetry_target_api.hpp"

// Implementation for smbus updateTelemetry
int tal::SmBusModule::updateTelemetry(
    const std::string& devicePath, const std::string& interface,
    const std::string& propName, std::vector<uint8_t>& rawData,
    const uint64_t timestamp, int rc,
    nv::sensor_aggregation::DbusVariantType& value,
    const std::string associatedEntityPath)
{
    if (rawData.size())
    {
        return updateSmbusTelemetry(devicePath, interface, propName, rawData,
                                    timestamp, rc);
    }
    return 0;
}

std::vector<nv::shmem::SensorValue>
    tal::SmBusModule::getAllMrds(const std::string& mrdNamespace)
{
    return {};
}

bool tal::SmBusModule::namespaceInit(const std::string& processName)
{
    // Implementation for smbus namespaceInit
    lg2::info(
        "Initializing smbus namespaceInit with process name:{PROCESSNAME}",
        "PROCESSNAME", processName);
    return smbusSlaveInit();
}

#endif
