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



#include "tal.hpp"
#include "tal_module.hpp"
#include "tal_smbus.hpp"
#include "tal_shmem.hpp"

using namespace tal;

std::unordered_map<TalModule, std::unique_ptr<TelemetryHandler>> modules;

TalModule talModulesArray[] = {
  TalModule::SharedMem,
#ifdef ENABLE_SMBUS
  TalModule::Smbus
#endif
};

bool talInit = false;

static void initTal() {
  // Iterate over each namespace in the talModule enum class
  for (TalModule talModule : talModulesArray) {
    switch (talModule) {
#ifdef ENABLE_SMBUS
      case TalModule::Smbus:
        modules.emplace(talModule, std::make_unique<SmBusModule>());
        break;
#endif
      case TalModule::SharedMem:
        modules.emplace(talModule, std::make_unique<SharedMemModule>());
        break;
      default:
        // Handle unknown namespaces
        lg2::error("unknown talModule");
        break;
    }
  }
  talInit = true;
}


void TelemetryAggregator::updateTelemetry(const std::string& devicePath,
                                          const std::string& interface,
                                          const std::string& propName,
                                          std::vector<uint8_t>& rawData,
                                          const uint64_t timestamp, int rc,
                                          nv::sensor_aggregation::DbusVariantType& value,
                                          const std::string associatedEntityPath) {
  if(!talInit){
    lg2::error("namespaceInit for tal is not invoked");
    return;
  }

  for (auto& [ns, module] : modules) {
    module->updateTelemetry(devicePath, interface, propName, rawData, timestamp, rc, value, associatedEntityPath);
  }
}

std::vector<nv::shmem::SensorValue> TelemetryAggregator::getAllMrds(const std::string& mrdNamespace) {
  if(!talInit){
    lg2::error("namespaceInit for tal is not invoked");
    return {};
  }

  for (auto& [talModule, module] : modules) {
    if (talModule == TalModule::SharedMem) {
      return module->getAllMrds(mrdNamespace);
    }
  }
  return {};
}

std::vector<std::string> TelemetryAggregator::getMrdNamespaces() {
  if(!talInit){
    lg2::error("namespaceInit for tal is not invoked");
    return {};
  }
  for (auto& [talModule, module] : modules) {
    if (talModule == TalModule::SharedMem) {
      return module->getMrdNamespaces();
    }
  }
  return {};
}

bool TelemetryAggregator::namespaceInit(ProcessType type, const std::string& processName) {
  if(talInit){
    lg2::error("namespaceInit for tal is already invoked");
    return false;
  }
  lg2::info("init tal");
  initTal();
  bool ret = true; // return value of the api to handle the error cases.
  if (type == ProcessType::Producer) {
    for (auto& [ns, module] : modules) {
      bool namespaceInitRet = module->namespaceInit(processName);
      ret &= namespaceInitRet;
    }
  }
  return ret;
}

