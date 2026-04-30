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

/*
 * Unit tests for TelemetryAggregator (TAL core).
 *
 * The file-static `talInit` flag in tal.cpp cannot be reset between tests,
 * so tests are grouped and ordered to flow through states:
 *
 *   TalPreInit   – talInit == false (runs first in a fresh process)
 *   TalInit      – transitions talInit to true via namespaceInit
 *   TalPostInit  – talInit == true, modules registered with stubs
 *
 * SharedMemModule and SmBusModule are replaced by no-op stubs
 * (tal_shmem_stub.cpp / tal_smbus_stub.cpp) so there is no runtime
 * dependency on nv::shmem or real SMBus devices.
 */

#include "tal.hpp"

#include <gtest/gtest.h>

using namespace tal;

// =========================================================================
// Pre-init tests  (talInit starts as false in a fresh process)
// MUST run before any test calls namespaceInit.
// =========================================================================

TEST(TalPreInit, UpdateTelemetryBeforeInitIsNoOp)
{
    std::vector<uint8_t> rawData;
    nv::sensor_aggregation::DbusVariantType val;
    EXPECT_NO_THROW(TelemetryAggregator::updateTelemetry(
        "/xyz/sensor", "xyz.Sensor.Value", "Value", rawData, 0ULL, 0, val));
}

TEST(TalPreInit, UpdateAggregateTelemetryBeforeInitIsNoOp)
{
    std::vector<TelemetryData> data;
    EXPECT_NO_THROW(TelemetryAggregator::updateAggregateTelemetry(data));
}

TEST(TalPreInit, GetAllMrdsBeforeInitReturnsEmpty)
{
    auto result = TelemetryAggregator::getAllMrds("test-namespace");
    EXPECT_TRUE(result.empty());
}

TEST(TalPreInit, GetMrdNamespacesBeforeInitReturnsEmpty)
{
    auto result = TelemetryAggregator::getMrdNamespaces();
    EXPECT_TRUE(result.empty());
}

TEST(TalPreInit, RepeatCallsBeforeInitDoNotCrash)
{
    // Exercises the repeatFailCount throttling guard (> 5 calls).
    std::vector<uint8_t> rawData;
    nv::sensor_aggregation::DbusVariantType val;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_NO_THROW(TelemetryAggregator::updateTelemetry(
            "/xyz/sensor", "xyz.Sensor.Value", "Value", rawData, 0ULL, 0, val));
    }
}

TEST(TalPreInit, RepeatAggregateTelemetryCallsBeforeInitDoNotCrash)
{
    std::vector<TelemetryData> data;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_NO_THROW(TelemetryAggregator::updateAggregateTelemetry(data));
    }
}

// =========================================================================
// Init tests — call namespaceInit for the first (and only) time
// =========================================================================

TEST(TalInit, NamespaceInitAsClientReturnsTrue)
{
    // Client ProcessType: initTal() creates module instances but does NOT
    // call module->namespaceInit(), so no external I/O occurs.
    bool result = TelemetryAggregator::namespaceInit(ProcessType::Client,
                                                     "test-proc");
    EXPECT_TRUE(result);
}

TEST(TalInit, DoubleNamespaceInitReturnsFalse)
{
    // talInit is now true from the previous test; a second call must fail.
    bool result = TelemetryAggregator::namespaceInit(ProcessType::Client,
                                                     "test-proc");
    EXPECT_FALSE(result);
}

// =========================================================================
// Post-init tests — talInit is true, stub modules are registered
// =========================================================================

TEST(TalPostInit, GetMrdNamespacesReturnsStubbedValue)
{
    // SharedMemModule stub returns {"stub-namespace"}
    auto result = TelemetryAggregator::getMrdNamespaces();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "stub-namespace");
}

TEST(TalPostInit, GetAllMrdsDoesNotCrash)
{
    // Stub returns empty vector; just verify no throw
    EXPECT_NO_THROW(TelemetryAggregator::getAllMrds("any-namespace"));
}

TEST(TalPostInit, UpdateTelemetryDelegatesToModules)
{
    std::vector<uint8_t> rawData = {0x01, 0x02};
    nv::sensor_aggregation::DbusVariantType val;
    EXPECT_NO_THROW(TelemetryAggregator::updateTelemetry(
        "/xyz/sensor", "xyz.Sensor.Value", "Value", rawData, 12345ULL, 0, val));
}

TEST(TalPostInit, UpdateTelemetryWithNonZeroRcDelegatesToModules)
{
    std::vector<uint8_t> rawData;
    nv::sensor_aggregation::DbusVariantType val;
    // rc != 0 signals a NaN/error update — stub should handle gracefully
    EXPECT_NO_THROW(TelemetryAggregator::updateTelemetry(
        "/xyz/sensor", "xyz.Sensor.Value", "Value", rawData, 12345ULL, 1, val));
}

TEST(TalPostInit, UpdateAggregateTelemetryWithDataDelegatesToModules)
{
    std::vector<TelemetryData> data(2);
    data[0].devicePath = "/xyz/sensor/0";
    data[0].interface = "xyz.Sensor.Value";
    data[0].propName = "Value";
    data[0].rc = 0;
    data[0].timestamp = 100ULL;

    data[1] = data[0];
    data[1].devicePath = "/xyz/sensor/1";

    EXPECT_NO_THROW(TelemetryAggregator::updateAggregateTelemetry(data));
}

TEST(TalPostInit, UpdateAggregateTelemetryWithEmptyDataDoesNotCrash)
{
    std::vector<TelemetryData> empty;
    EXPECT_NO_THROW(TelemetryAggregator::updateAggregateTelemetry(empty));
}

TEST(TalPostInit, UpdateTelemetryWithAssociatedEntityPath)
{
    std::vector<uint8_t> rawData = {0xAB};
    nv::sensor_aggregation::DbusVariantType val;
    EXPECT_NO_THROW(TelemetryAggregator::updateTelemetry(
        "/xyz/sensor", "xyz.Sensor.Value", "Value", rawData, 99ULL, 0, val,
        "/xyz/chassis/0"));
}
