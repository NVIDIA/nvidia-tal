#include "tal_module.hpp"

#ifdef ENABLE_SMBUS
namespace tal {
class SmBusModule : public TelemetryHandler {
  public:
    int updateTelemetry(const std::string& devicePath,
                        const std::string& interface,
                        const std::string& propName,
                        std::vector<uint8_t>& rawData,
                        const uint64_t timestamp, int rc,
                        nv::sensor_aggregation::DbusVariantType& value,
                        const std::string associatedEntityPath = {}) override;

    std::vector<nv::shmem::SensorValue> getAllMrds(const std::string& mrdNamespace) override;

    bool namespaceInit(const std::string& processName) override;

    virtual ~SmBusModule() = default;
};
}
#endif
