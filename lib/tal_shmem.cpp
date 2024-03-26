#include "tal_shmem.hpp"
#include <telemetry_mrd_producer.hpp>
#include <telemetry_mrd_client.hpp>

int tal::SharedMemModule::updateTelemetry(const std::string& devicePath,
                                          const std::string& interface,
                                          const std::string& propName,
                                          std::vector<uint8_t>& rawData,
                                          const uint64_t timestamp, int rc,
                                          nv::sensor_aggregation::DbusVariantType& value,
                                          const std::string associatedEntityPath) {
  return nv::shmem::AggregationService::updateTelemetry(devicePath,
                                                        interface,
                                                        propName,
                                                        value,
                                                        timestamp, rc,
                                                        associatedEntityPath);
}

bool tal::SharedMemModule::namespaceInit(const std::string& processName) {
  lg2::info("Initializing shmem namespaceInit with process name:{PROCESSNAME}","PROCESSNAME", processName);
  return nv::shmem::AggregationService::namespaceInit(processName);
}

std::vector<nv::shmem::SensorValue> tal::SharedMemModule::getAllMrds(const std::string& mrdNamespace) {
  return nv::shmem::sensor_aggregation::getAllMRDValues(mrdNamespace);
}
