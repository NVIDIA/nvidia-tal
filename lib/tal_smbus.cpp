#include "tal_smbus.hpp"

#ifdef ENABLE_SMBUS
#include "smbus_telemetry_target_api.hpp"

// Implementation for smbus updateTelemetry
int tal::SmBusModule::updateTelemetry(const std::string& devicePath,
                                      const std::string& interface,
                                      const std::string& propName,
                                      std::vector<uint8_t>& rawData,
                                      const uint64_t timestamp,
                                      int rc,
                                      nv::sensor_aggregation::DbusVariantType& value,
                                      const std::string associatedEntityPath) {
  if (rawData.size()){
    return updateSmbusTelemetry(devicePath, interface, propName, rawData, timestamp, rc);
  }
  return 0;
}

std::vector<nv::shmem::SensorValue> tal::SmBusModule::getAllMrds(const std::string& mrdNamespace) {
  return {};
}

bool tal::SmBusModule::namespaceInit(const std::string& processName) {
  // Implementation for smbus namespaceInit
  lg2::info("Initializing smbus namespaceInit with process name:{PROCESSNAME}","PROCESSNAME", processName);
  return smbusSlaveInit();

}

#endif
