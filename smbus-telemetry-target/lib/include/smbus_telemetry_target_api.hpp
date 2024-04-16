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

#include <cstdint>
#include <string>
#include <vector>

/*
 * @brief  The init API is used by the library to pre-populate all the meta data
 * necessary from data files and setup the data structures once on start up.
 * @return If init successfully it will return true else false
 */

bool smbusSlaveInit();

/*
 * @brief The update API will be used by all the sensor producers to regularly
 * refresh the values exposed on smbus slave.
 * @para1 DevicePath is sensor dbus object path
 * @para2 Interface is sensor dbus interface
 * @para3 PropName is sensor proeprty
 * @para4 raw data of the sensor from protocols - SMBPBI/PLDM/NSM etc
 * @para5 dataLength is length for raw value
 * @para6 Timestamp is last refresh time
 * @para7 Rc is return code of the sensor API
 *
 * @return It return zero for success else error RC
 */
int updateSmbusTelemetry(const std::string& devicePath,
                         const std::string& interface,
                         const std::string& propName, std::vector<uint8_t>& data,
                         const uint64_t timestamp, int rc);
