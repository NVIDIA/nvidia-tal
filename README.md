# Library for Shared Memory based Sensor Aggregation

Author: Deepak kamat <dkamat@nvidia.com> Rohit Pai <ropai@nvidia.com>

Created: 2024-04-04

## Problem Description

In the current architecture we have sensor aggregation APIs which are based on shared memory infrastructure. Due to this it is very tightly coupled with the changes if any other telemetry module is introduced we need to add the api on each producer and client side. So adding Tal will create a layer.

## Background and References
TAL APIs would be inserted in telemetry producers and telemetry readers so that these applications are not impacted by changes in the infrastructure libraries or IPC mechanisms. At present the scope of TAL API is limited to accessing MRD telemetry objects.

## Requirements

### External API requirements
- API for data producer to update the data from the tal modules.
- API for client such as bmcweb to get the data from the tal modules .

### Telemetry Handler Interface

- Each telemetry module must implement a telemetry handler interface and register with the TAL framework. 
- Everytime TAL public API is triggered, TAL will loop over the registered modules and call the equivalent interface APIs. 
- Any locking necessary to update the telemetry in the database must be implemented by the telemetry module. 
- Any optimization required to selectively update some telemetry in the database and ignore others must be implemented by the telemetry module. 

### Update Telemetry Flow

- All the producers will call the update telemrty from the Tal public interface.
- Tal will update all the modules that are listed with Tal.

### Read Telemetry Flow
- All the client will call the get telemrty from the Tal public interface.
- Currently we are reading from shared memory only.

### TAL APIs

#### Init namespace
Method to initialize namespace specific operations. Interface to initialize the namespace for module.

API:

```ascii
static bool namespaceInit(ProcessType type, const std::string& processName = {});
```

Parameters:

- processName - process name of producer service
- ProcessType type - process type can be client or producer.
- return - gives false if any of the namespaceinit fails.

#### Update telemetry

Method to update telemetry for all modules.

API:

```ascii
static void updateTelemetry(const std::string& devicePath,
                                const std::string& interface,
                                const std::string& propName,
                                std::vector<uint8_t>& rawData,
                                const uint64_t timestamp, int rc,
                                nv::sensor_aggregation::DbusVariantType& value,
                                const std::string associatedEntityPath = {});
```

Parameters:

- devicePath - Device path of telemetry object.
- interface - Phosphor D-Bus interface of telemetry object.
- propName - Metric name.
- rawData - raw data of the value.
- value - Metric value.
- timestamp - Timestamp of telemetry object.
- rc - Set this value to non zero for nan update.
- associatedEntityPath - optional for other metrics. Required for platform environment metrics.

#### Read telemetry
This API returns all metric report definitions for a shared mem namespace others are skipped

API:

```ascii
static std::vector<nv::shmem::SensorValue> getAllMrds(const std::string& mrdNamespace);
```

Parameters:
- mrdNamespace - metric report definitions namespace.

#### Get Namespaces.

Method to get all metric report collection from the shmem

API:

```ascii
static std::vector<std::string> getMrdNamespaces();
```

