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

#include "error.hpp"
#include "smbus_telemetry_target_api.hpp"
#include "smbus_telemetry_update.hpp"

#include <cstdio>
#include <fstream>

#include "gmock/gmock.h"
#include <gtest/gtest.h>

using namespace std;

// -------------------------------------------------------------------------
// Extern declarations for internal globals accessed by test fixtures.
// These are defined in the respective .cpp files and are accessed here to
// reset state between tests.
// -------------------------------------------------------------------------
extern bool smbusTelemetryInit;

namespace smbus_telemetry_update
{
extern smbusSensorMap sensorDataMap;
extern uint64_t slaveI2cStaleThresholdMs;
} // namespace smbus_telemetry_update

// -------------------------------------------------------------------------
// Helper: insert a sensor entry directly into sensorDataMap.
// -------------------------------------------------------------------------
static void addSensorEntry(const std::string& objPath, const std::string& iface,
                           const std::string& prop, uint16_t offset = 0,
                           int length = 4, int staleOffset = -1,
                           int staleBit = -1)
{
    smbus_telemetry_update::SmbusSensorData sd;
    sd.setSensorOffset(offset);
    sd.setOffsetDataLength(length);
    sd.setDbusObjPath(objPath);
    sd.setDbusIface(iface);
    sd.setDbusProperty(prop);
    sd.setStaleOffset(staleOffset);
    sd.setStaleBit(staleBit);
    std::string key = objPath + "_" + iface + "_" + prop;
    smbus_telemetry_update::sensorDataMap.emplace(key, sd);
}

// -------------------------------------------------------------------------
// Test fixture: resets all mutable global state before each test.
// -------------------------------------------------------------------------
class SmbusTelemetryTest : public ::testing::Test
{
  protected:
    static constexpr const char* tmpEepromPath = "/tmp/smbus_eeprom_test";
    const char* savedSysfsPath = nullptr;

    void SetUp() override
    {
        smbus_telemetry_update::sensorDataMap.clear();
        while (!smbus_telemetry_update::smbusSensorDataQueue.empty())
            smbus_telemetry_update::smbusSensorDataQueue.pop();
        smbus_telemetry_update::slaveI2cStaleThresholdMs = 0;
        smbusTelemetryInit = false;
        savedSysfsPath = smbus_telemetry_update::i2cSlaveSysfs;
    }

    void TearDown() override
    {
        smbus_telemetry_update::i2cSlaveSysfs = savedSysfsPath;
        smbus_telemetry_update::sensorDataMap.clear();
        while (!smbus_telemetry_update::smbusSensorDataQueue.empty())
            smbus_telemetry_update::smbusSensorDataQueue.pop();
        std::remove(tmpEepromPath);
    }

    // Creates a zeroed temp file and redirects i2cSlaveSysfs to it.
    void createTempEeprom(size_t size = 64)
    {
        std::ofstream f(tmpEepromPath,
                        std::ios::binary | std::ios::out | std::ios::trunc);
        std::vector<uint8_t> zeros(size, 0);
        f.write(reinterpret_cast<const char*>(zeros.data()),
                static_cast<std::streamsize>(size));
        smbus_telemetry_update::i2cSlaveSysfs = tmpEepromPath;
    }
};

TEST(SmbusTelemetryUpdateApi_1, loadFromCSV)
{
    // Test case for init failure
    int rc = smbus_telemetry_update::loadFromCSV("./wrong-fileName.csv");
    EXPECT_EQ(rc, 0x0100);

    // Invalid data in row1 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row1-failure.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row2 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row2-failure.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong offset field on  csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-offset.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong length field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-length.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong data_format field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-dataformat.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong dbus_object field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-dbusobjpath.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong dbus_interface field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-dbusiface.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong data_property field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-dbusproperty.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong stale_offset field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-stalebit.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row3 wrong stale_bit field on csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row3-failure-wrong-staleoffset.csv");
    EXPECT_EQ(rc, 0x0101);

    // Invalid data in row4 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/row4-failure.csv");
    EXPECT_EQ(rc, 0x0101);

    // Test case for init success
    rc = smbus_telemetry_update::loadFromCSV(
        "smbus-telemetry-target/test/config/smbus-telemetry-config.csv");
    EXPECT_EQ(rc, 0);
}

TEST(SmbusTelemetryUpdateApi_2, smbusSlaveUpdate)
{
    // Test case for smbusSlaveUpdate
    std::string dbusObjPath = "tmp_objpath";
    std::string iface = "tmp_iface";
    std::string propName = "tmp_propertyname";
    std::vector<uint8_t> val = {};
    uint64_t ts = 0;
    int retVal = 0;
    // call smbusSlaveUpdate with wrong data
    int rc = smbus_telemetry_update::smbusSlaveUpdate(
        dbusObjPath, iface, propName, val, ts, retVal);
    EXPECT_EQ(rc, 0);
}

TEST(SmbusTelemetryTargetApi_1, smbusSlaveInit)
{
    // Default smbus device is /dev/null
    // So it should fail
    bool status = smbusSlaveInit();
    EXPECT_EQ(status, false);
}

// =========================================================================
// SmbusSensorData — class interface tests
// =========================================================================

TEST(SmbusSensorDataTest, DefaultConstructorValues)
{
    smbus_telemetry_update::SmbusSensorData sd;
    EXPECT_EQ(sd.getOffsetDataLength(), 0);
    EXPECT_EQ(sd.getSensorOffset(), 0u);
    EXPECT_EQ(sd.previousTimeStamp, 0u);
    EXPECT_TRUE(sd.initTs);
}

TEST(SmbusSensorDataTest, GettersSettersRoundTrip)
{
    smbus_telemetry_update::SmbusSensorData sd;

    sd.setOffsetDataLength(8);
    EXPECT_EQ(sd.getOffsetDataLength(), 8);

    sd.setSensorOffset(0x10);
    EXPECT_EQ(sd.getSensorOffset(), 0x10u);

    sd.setStaleOffset(0x20);
    EXPECT_EQ(sd.getStaleOffset(), 0x20);

    sd.setStaleBit(3);
    EXPECT_EQ(sd.getStaleBit(), 3);

    sd.setDbusObjPath("/xyz/obj");
    EXPECT_EQ(sd.getDbusObjPath(), "/xyz/obj");

    sd.setDbusIface("xyz.Sensor");
    EXPECT_EQ(sd.getDbusIface(), "xyz.Sensor");

    sd.setDbusProperty("Value");
    EXPECT_EQ(sd.getDbusProperty(), "Value");
}

TEST(SmbusSensorDataTest, StaleOffsetAndBitNegativeValues)
{
    smbus_telemetry_update::SmbusSensorData sd;
    sd.setStaleOffset(-1);
    EXPECT_EQ(sd.getStaleOffset(), -1);
    sd.setStaleBit(-1);
    EXPECT_EQ(sd.getStaleBit(), -1);
}

// =========================================================================
// smbusSlaveUpdate — additional path coverage
// =========================================================================

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_KeyNotInMap_ReturnsZero)
{
    // sensorDataMap is empty; key lookup misses → returns 0
    std::vector<uint8_t> val = {0x01, 0x02};
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 100ULL, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_EepromFileNotFound_ReturnsError)
{
    addSensorEntry("obj", "iface", "prop", 0, 4);
    smbus_telemetry_update::i2cSlaveSysfs = "/nonexistent/path/eeprom";
    std::vector<uint8_t> val = {0x01, 0x02, 0x03, 0x04};
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 100ULL, 0);
    EXPECT_EQ(rc, ErrorCode::SMBusSysfsPathNotFound);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_SuccessfulWrite_ReturnsZero)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);
    std::vector<uint8_t> val = {0xAA, 0xBB, 0xCC, 0xDD};
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 100ULL, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_RcNonZero_WritesFfBytes)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);
    std::vector<uint8_t> val = {0x01, 0x02, 0x03, 0x04};
    // rc != 0 → value is replaced with 0xFF bytes before writing
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 100ULL, 1);
    EXPECT_EQ(rc, 0);

    // Verify 0xFF was written at offset 0 (length 4)
    std::ifstream f(tmpEepromPath, std::ios::binary);
    uint8_t buf[4] = {};
    f.read(reinterpret_cast<char*>(buf), 4);
    EXPECT_EQ(buf[0], 0xFF);
    EXPECT_EQ(buf[1], 0xFF);
    EXPECT_EQ(buf[2], 0xFF);
    EXPECT_EQ(buf[3], 0xFF);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_InitTs_FirstCallStoresTsAndClears)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);
    std::vector<uint8_t> val = {0x01};

    // First call: initTs == true → timestamp stored, initTs cleared
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 1000ULL, 0);
    EXPECT_EQ(rc, 0);

    std::string key = "obj_iface_prop";
    EXPECT_FALSE(smbus_telemetry_update::sensorDataMap.at(key).initTs);
    EXPECT_EQ(smbus_telemetry_update::sensorDataMap.at(key).previousTimeStamp,
              1000ULL);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_SecondCallUpdatesPreviousTs)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);
    std::vector<uint8_t> val = {0x01};

    // First call initialises the timestamp
    smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop", val,
                                             1000ULL, 0);
    // Second call updates previousTimeStamp
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 2000ULL, 0);
    EXPECT_EQ(rc, 0);

    std::string key = "obj_iface_prop";
    EXPECT_EQ(smbus_telemetry_update::sensorDataMap.at(key).previousTimeStamp,
              2000ULL);
}

TEST_F(SmbusTelemetryTest,
       SmbusSlaveUpdate_StaleDetection_DeltaExceedsThreshold)
{
    createTempEeprom(64);
    smbus_telemetry_update::slaveI2cStaleThresholdMs = 500;
    addSensorEntry("obj", "iface", "prop", 0, 4, 8, 0);
    std::vector<uint8_t> val = {0x01};

    // First call: initTs path — no stale computed
    smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop", val,
                                             1000ULL, 0);
    // Second call: delta = 2000 - 1000 = 1000 > 500 → stale == 1
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 2000ULL, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_NoStaleBitConfigured_SkipsStalePath)
{
    createTempEeprom(64);
    // staleOffset and staleBit both -1 (NA in CSV)
    addSensorEntry("obj", "iface", "prop", 0, 4, -1, -1);
    std::vector<uint8_t> val = {0xDE, 0xAD, 0xBE, 0xEF};
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 100ULL, 0);
    EXPECT_EQ(rc, 0);
}

// =========================================================================
// updateSmbusTelemetry — init-flag gating
// =========================================================================

TEST_F(SmbusTelemetryTest, UpdateSmbusTelemetry_NotInit_ReturnsZeroImmediately)
{
    smbusTelemetryInit = false;
    std::vector<uint8_t> val = {0x01};
    int rc = updateSmbusTelemetry("obj", "iface", "prop", val, 100ULL, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(SmbusTelemetryTest, UpdateSmbusTelemetry_Init_KeyNotInMap_ReturnsZero)
{
    smbusTelemetryInit = true;
    // sensorDataMap is empty → smbusSlaveUpdate returns 0
    std::vector<uint8_t> val = {0x01};
    int rc = updateSmbusTelemetry("obj", "iface", "prop", val, 100ULL, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(SmbusTelemetryTest, UpdateSmbusTelemetry_Init_KeyFound_WritesToEeprom)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);
    smbusTelemetryInit = true;
    std::vector<uint8_t> val = {0x11, 0x22, 0x33, 0x44};
    int rc = updateSmbusTelemetry("obj", "iface", "prop", val, 200ULL, 0);
    EXPECT_EQ(rc, 0);
}

// =========================================================================
// updateSmbusAggregateTelemetry — delegation check
// =========================================================================

TEST_F(SmbusTelemetryTest, UpdateSmbusAggregateTelemetry_EmptyData_NoOp)
{
    std::vector<tal::TelemetryData> data;
    EXPECT_NO_THROW(updateSmbusAggregateTelemetry(data));
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest,
       UpdateSmbusAggregateTelemetry_KeyNotInMap_QueueStaysEmpty)
{
    std::vector<tal::TelemetryData> data(1);
    data[0].devicePath = "obj";
    data[0].interface = "iface";
    data[0].propName = "prop";
    EXPECT_NO_THROW(updateSmbusAggregateTelemetry(data));
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

// =========================================================================
// smbusSlaveUpdateAggregate / flushBufferToEeprom
// =========================================================================

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdateAggregate_EmptyData_NoOp)
{
    std::vector<tal::TelemetryData> data;
    smbus_telemetry_update::smbusSlaveUpdateAggregate(data);
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdateAggregate_KeyNotInMap_Skipped)
{
    std::vector<tal::TelemetryData> data(1);
    data[0].devicePath = "obj";
    data[0].interface = "iface";
    data[0].propName = "prop";
    smbus_telemetry_update::smbusSlaveUpdateAggregate(data);
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdateAggregate_KeyFound_QueuedAndFlushed)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);

    std::vector<tal::TelemetryData> data(1);
    data[0].devicePath = "obj";
    data[0].interface = "iface";
    data[0].propName = "prop";
    data[0].rawData = {0x01, 0x02, 0x03, 0x04};
    data[0].timestamp = 100ULL;
    data[0].rc = 0;

    smbus_telemetry_update::smbusSlaveUpdateAggregate(data);
    // flushBufferToEeprom is called internally; queue must be empty after
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest, FlushBufferToEeprom_EmptyQueue_IsNoOp)
{
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
    EXPECT_NO_THROW(smbus_telemetry_update::flushBufferToEeprom());
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest, FlushBufferToEeprom_FileNotFound_DrainQueue)
{
    addSensorEntry("obj", "iface", "prop", 0, 4);
    smbus_telemetry_update::i2cSlaveSysfs = "/nonexistent/eeprom";

    std::string key = "obj_iface_prop";
    smbus_telemetry_update::smbusSensorDataQueue.push(
        std::make_tuple(&smbus_telemetry_update::sensorDataMap.at(key),
                        std::vector<uint8_t>{0x01}, 100ULL, 0));

    smbus_telemetry_update::flushBufferToEeprom();
    // Queue must be drained even when the file cannot be opened
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

TEST_F(SmbusTelemetryTest, FlushBufferToEeprom_ValidFile_WritesAndDrainsQueue)
{
    createTempEeprom(64);
    addSensorEntry("obj", "iface", "prop", 0, 4);

    std::string key = "obj_iface_prop";
    smbus_telemetry_update::smbusSensorDataQueue.push(std::make_tuple(
        &smbus_telemetry_update::sensorDataMap.at(key),
        std::vector<uint8_t>{0xCA, 0xFE, 0xBA, 0xBE}, 200ULL, 0));

    smbus_telemetry_update::flushBufferToEeprom();
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

// =========================================================================
// smbusSlaveUpdate — stale=0 branch with stale config
// =========================================================================

TEST_F(SmbusTelemetryTest, SmbusSlaveUpdate_StaleConfigured_StaleZero_ClearsBit)
{
    // staleOffset=8, staleBit=0, threshold large → stale=0 on first call
    // (initTs path sets previousTimeStamp=ts, then delta=0 ≤ threshold →
    // stale=0) Exercises the else branch: existingStaleValue &= ~(1 <<
    // staleBit)
    createTempEeprom(16);
    smbus_telemetry_update::slaveI2cStaleThresholdMs = 5000;
    addSensorEntry("obj", "iface", "prop", 0, 4, 8, 0);
    std::vector<uint8_t> val = {0x01, 0x02, 0x03, 0x04};
    int rc = smbus_telemetry_update::smbusSlaveUpdate("obj", "iface", "prop",
                                                      val, 1000ULL, 0);
    EXPECT_EQ(rc, 0);
}

// =========================================================================
// flushBufferToEeprom — stale paths with stale config
// =========================================================================

TEST_F(SmbusTelemetryTest, FlushBufferToEeprom_RcNonZero_WithStaleConfig)
{
    // rc!=0 → !success → stale=1, value filled with 0xFF
    // staleOffset=8 → enters stale block → if(stale) → sets bit
    createTempEeprom(16);
    addSensorEntry("obj", "iface", "prop", 0, 4, 8, 0);

    std::string key = "obj_iface_prop";
    smbus_telemetry_update::smbusSensorDataQueue.push(std::make_tuple(
        &smbus_telemetry_update::sensorDataMap.at(key),
        std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04}, 100ULL, 1));

    smbus_telemetry_update::flushBufferToEeprom();
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());

    // Verify 0xFF bytes were written at sensor offset 0
    std::ifstream f(tmpEepromPath, std::ios::binary);
    uint8_t buf[4] = {};
    f.read(reinterpret_cast<char*>(buf), 4);
    EXPECT_EQ(buf[0], 0xFF);
    EXPECT_EQ(buf[3], 0xFF);
}

TEST_F(SmbusTelemetryTest, FlushBufferToEeprom_StaleConfig_StaleZero)
{
    // initTs=true → previousTimeStamp=ts after first call, delta=0 → stale=0
    // staleOffset=8 → enters stale block → else → clears bit
    createTempEeprom(16);
    smbus_telemetry_update::slaveI2cStaleThresholdMs = 5000;
    addSensorEntry("obj", "iface", "prop", 0, 4, 8, 0);

    std::string key = "obj_iface_prop";
    smbus_telemetry_update::smbusSensorDataQueue.push(std::make_tuple(
        &smbus_telemetry_update::sensorDataMap.at(key),
        std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04}, 1000ULL, 0));

    smbus_telemetry_update::flushBufferToEeprom();
    EXPECT_TRUE(smbus_telemetry_update::smbusSensorDataQueue.empty());
}

// =========================================================================
// smbusSlaveInit — non-/dev/null path
// =========================================================================

TEST_F(SmbusTelemetryTest, SmbusSlaveInit_NonDevNull_CsvLoadFails)
{
    // i2cSlaveSysfs != "/dev/null" → passes the strcmp check
    // loadFromCSV(SMBUS_SLAVE_TELEMETRY_CONFIG_CSV) fails (file absent)
    // → rc != 0 → smbusSlaveInit returns false
    smbus_telemetry_update::i2cSlaveSysfs = "/tmp/fake_sysfs_device";
    bool status = smbusSlaveInit();
    EXPECT_FALSE(status);
    // smbusTelemetryInit must remain false on failure
    EXPECT_FALSE(smbusTelemetryInit);
}

// =========================================================================
// updateSmbusTelemetry — error return path (retVal != 0)
// =========================================================================

TEST_F(SmbusTelemetryTest, UpdateSmbusTelemetry_SmbusUpdateError_ReturnsError)
{
    // smbusTelemetryInit=true, key in map, eeprom missing
    // → smbusSlaveUpdate returns SMBusSysfsPathNotFound
    // → retVal != 0 → return retVal
    smbusTelemetryInit = true;
    addSensorEntry("obj", "iface", "prop", 0, 4);
    smbus_telemetry_update::i2cSlaveSysfs = "/nonexistent/eeprom";
    std::vector<uint8_t> val = {0x01};
    int rc = updateSmbusTelemetry("obj", "iface", "prop", val, 100ULL, 0);
    EXPECT_EQ(rc, ErrorCode::SMBusSysfsPathNotFound);
}
