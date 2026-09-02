// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataPacking.h"

#include "HMAPI.h"
#include "HMDataPacking.h"
#include "HMHashMD5.h"
#include "common.h"
#include "netchasm/hashinfo.pb.h"

#include <map>
#include <string>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

namespace
{
unique_ptr<char[]> serializeHashPair(const netchasm::HashHGPair& pair, uint64_t& dataSize)
{
    dataSize = pair.ByteSize();
    unique_ptr<char[]> data = make_unique<char[]>(dataSize);
    pair.SerializeToArray(data.get(), dataSize);
    return data;
}

unique_ptr<char[]> serializeHashInfo(const netchasm::HashInfo& info, uint64_t& dataSize)
{
    dataSize = info.ByteSize();
    unique_ptr<char[]> data = make_unique<char[]>(dataSize);
    info.SerializeToArray(data.get(), dataSize);
    return data;
}
} // namespace

void TESTNAME::setUp()
{
    setupCommon();
}

void TESTNAME::tearDown()
{
    teardownCommon();
}

void TESTNAME::TestOversizedHashRejected()
{
    HMDataPacking packing;
    const size_t oversize = HASH_MAX_SIZE + 1;
    string bytes(oversize, 'A');

    netchasm::HashHGPair pair;
    pair.set_hostgroupname("oversize-hg");
    pair.set_size(static_cast<int32_t>(oversize));
    pair.set_hash(bytes.data(), bytes.size());

    uint64_t dataSize = 0;
    unique_ptr<char[]> data = serializeHashPair(pair, dataSize);

    HMHash hash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, hash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), hash.m_hashSize);

    HMAPIHash apiHash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, apiHash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), apiHash.m_hashSize);

    netchasm::HashInfo info;
    *info.add_items() = pair;
    uint64_t infoSize = 0;
    unique_ptr<char[]> infoData = serializeHashInfo(info, infoSize);
    map<string, HMAPIHash> apiInfo;
    CPPUNIT_ASSERT(!packing.unpackHashInfo(infoData, infoSize, apiInfo));
    CPPUNIT_ASSERT(apiInfo.empty());
}

void TESTNAME::TestMismatchedHashRejected()
{
    HMDataPacking packing;
    const size_t declaredSize = 10;
    string bytes("short");

    netchasm::HashHGPair pair;
    pair.set_hostgroupname("mismatch-hg");
    pair.set_size(static_cast<int32_t>(declaredSize));
    pair.set_hash(bytes.data(), bytes.size());

    uint64_t dataSize = 0;
    unique_ptr<char[]> data = serializeHashPair(pair, dataSize);

    HMHash hash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, hash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), hash.m_hashSize);

    HMAPIHash apiHash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, apiHash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), apiHash.m_hashSize);

    netchasm::HashInfo info;
    *info.add_items() = pair;
    uint64_t infoSize = 0;
    unique_ptr<char[]> infoData = serializeHashInfo(info, infoSize);
    map<string, HMAPIHash> apiInfo;
    CPPUNIT_ASSERT(!packing.unpackHashInfo(infoData, infoSize, apiInfo));
    CPPUNIT_ASSERT(apiInfo.empty());
}

void TESTNAME::TestNegativeHashSizeRejected()
{
    HMDataPacking packing;
    string bytes("neg");

    netchasm::HashHGPair pair;
    pair.set_hostgroupname("negative-hg");
    pair.set_size(-1);
    pair.set_hash(bytes.data(), bytes.size());

    uint64_t dataSize = 0;
    unique_ptr<char[]> data = serializeHashPair(pair, dataSize);

    HMHash hash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, hash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), hash.m_hashSize);

    HMAPIHash apiHash;
    CPPUNIT_ASSERT(!packing.unpackHash(data, dataSize, apiHash));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), apiHash.m_hashSize);

    netchasm::HashInfo info;
    *info.add_items() = pair;
    uint64_t infoSize = 0;
    unique_ptr<char[]> infoData = serializeHashInfo(info, infoSize);
    map<string, HMAPIHash> apiInfo;
    CPPUNIT_ASSERT(!packing.unpackHashInfo(infoData, infoSize, apiInfo));
    CPPUNIT_ASSERT(apiInfo.empty());
}
