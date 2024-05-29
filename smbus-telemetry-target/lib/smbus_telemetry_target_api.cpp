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

#include "smbus_telemetry_target_api.hpp"

#include "smbus_telemetry_update.hpp"

#include <cstring>
#include <iostream>

bool smbusTelemetryInit = false;

bool smbusSlaveInit()
{
    // Checking for slave device
    if (std::strcmp(smbus_telemetry_update::i2cSlaveSysfs, "/dev/null") == 0)
    {
        lg2::error("SMBus slave device not configured");
        return false;
    }

    int rc =
        smbus_telemetry_update::loadFromCSV(SMBUS_SLAVE_TELEMETRY_CONFIG_CSV);
    if (rc != 0)
    {
        lg2::error("Failed to load data from CSV");
        return false;
    }
    smbusTelemetryInit = true;
    return true;
}

int updateSmbusTelemetry(const std::string& devicePath,
                         const std::string& interface,
                         const std::string& propName,
                         std::vector<uint8_t>& data, const uint64_t timestamp,
                         int rc)
{
    // smbusSlaveInit not success telemetry update call return gracefully
    if (!smbusTelemetryInit)
        return 0;

    int retVal = smbus_telemetry_update::smbusSlaveUpdate(
        devicePath, interface, propName, data, timestamp, rc);
    if (retVal != 0)
    {
        return retVal;
    }

    return 0;
}
