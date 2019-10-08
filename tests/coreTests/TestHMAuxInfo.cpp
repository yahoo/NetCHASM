// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMAuxInfo.h"

#include "HMAuxCache.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{

}

void TESTNAME::tearDown()
{

}

void TESTNAME::test_HMAuxInfo_MalformedXML()
{
    // The tag <MF> is not closed making this XML invalid
    string testXML = " \
<load-object domain=\"test.hm.com\" xmlns=\"http://www.akamai.com/FirstPoint/load-balancing\" timestamp=\"2018-2-1T0:3:1Z\" version=\"1\">\n\
\t<datacenter name=\"d3\">\n\
\t\t<resource name=\"mra1-canary\">\n\
\t\t\t<hostname>mra1.canary.hm.com</hostname>\n\
\t\t\t<current-load>11</current-load>\n\
\t\t\t<target-load>2500</target-load>\n\
\t\t\t<capacity>5000</capacity>\n\
\t\t</resource>\n\
\t</datacenter>\n\
\t<MF>101\n\
\t<datacenter name=\"D3\">\n\
\t\t<resource name=\"api1_sats_cdn\">\n\
\t\t\t<hostname>api.hm.com</hostname>\n\
\t\t\t<current-load>3</current-load>\n\
\t\t\t<target-load>38</target-load>\n\
\t\t\t<capacity>76</capacity>\n\
\t\t</resource>\n\
\t\t<resource name=\"canl\">\n\
\t\t\t<hostname>api.hm.com</hostname>\n\
\t\t\t<current-load>118</current-load>\n\
\t\t\t<target-load>500</target-load>\n\
\t\t\t<capacity>2000</capacity>\n\
\t\t</resource>\n\
\t</datacenter>\n\
</load-object>\n";

    HMAuxCache auxInfo;
    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");

    CPPUNIT_ASSERT(!auxInfo.storeAuxInfo(hostName, sourceURL, address, testXML, HM_AUX_DATA_XML));
}


void TESTNAME::test_HMAuxInfo_NewLFB()
{
    string testXML = " \
    <load-file updated=\"1518198961\">\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <time>1518198900</time>\n\
    <load>485</load>\n\
    <target>2500</target>\n\
    <max>5000</max>\n\
    </host>\n\
    </resource>\n\
    <resource name=\"pr.ybp\">\n\
    <host name=\"pr.hm1.com\">\n\
    <time>1436935860</time>\n\
    <load>900</load>\n\
    <target>500</target>\n\
    <max>1000</max>\n\
    </host>\n\
    </resource>\n\
    <resource name=\"fam2\">\n\
    <host name=\"b1.hm2.com\">\n\
    <time>1518198900</time>\n\
    <load>3</load>\n\
    <target>100</target>\n\
    <max>150</max>\n\
    </host>\n\
    </resource>\n\
    </load-file>\n";
    HMAuxInfo auxInfo;
    HMAuxCache auxCache;
    std::vector<std::unique_ptr<HMAuxBase>> singleResult;
    HMAuxLoadFB* lfb;
    HMTimeStamp ts,ts1;
    ts.setTime(1518198900);
    ts1.setTime(1436935860);

    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;

    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXML, HM_AUX_DATA_XML));

    CPPUNIT_ASSERT(
            auxCache.getAuxInfo(hostName, sourceURL, address, auxInfo));
    CPPUNIT_ASSERT_EQUAL(3, (int)auxInfo.m_auxData.size());

    lfb = dynamic_cast<HMAuxLoadFB*>(auxInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(
            lfb->m_host == "mr1.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "mr1_prod");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_load == 485);
    CPPUNIT_ASSERT(lfb->m_target == 2500);
    CPPUNIT_ASSERT(lfb->m_max == 5000);

    lfb = dynamic_cast<HMAuxLoadFB*>(auxInfo.m_auxData[1].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "pr.hm1.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "pr.ybp");
    CPPUNIT_ASSERT(lfb->m_ts == ts1);
    CPPUNIT_ASSERT(lfb->m_load == 900);
    CPPUNIT_ASSERT(lfb->m_target == 500);
    CPPUNIT_ASSERT(lfb->m_max == 1000);

    lfb = dynamic_cast<HMAuxLoadFB*>(auxInfo.m_auxData[2].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "b1.hm2.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "fam2");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_load == 3);
    CPPUNIT_ASSERT(lfb->m_target == 100);
    CPPUNIT_ASSERT(lfb->m_max == 150);

    CPPUNIT_ASSERT(
            auxCache.genAuxData(auxInfo, HM_LOAD_FILE, "test.hm.com",
                    xmlOut, HM_AUX_DATA_XML));
}

void TESTNAME::test_parse_HMAuxInfo_NewLFB()
{
    string testXMLWoResources = " \
    <load-file updated=\"1518198961\">\n\
    </load-file>\n";
    string testXMLWoResourceNameAttr = " \
            <load-file updated=\"1518198961\">\n\
            <resource>\n\
            </resource>\n\
            </load-file>\n";

    string testXMLWoHostName = " \
            <load-file updated=\"1518198961\">\n\
            <resource name=\"mr1_prod\">\n\
            </resource>\n\
            </load-file>\n";
    string testXMLWoHostNameAttr = " \
            <load-file updated=\"1518198961\">\n\
            <resource name=\"mr1_prod\">\n\
            <host>\n\
            </host>\n\
            </resource>\n\
            </load-file>\n";
    string testXMLWoLoad = " \
            <load-file updated=\"1518198961\">\n\
            <resource name=\"mr1_prod\">\n\
            <host name=\"mr1.hm.com\">\n\
            </host>\n\
            </resource>\n\
            </load-file>\n";
    string testXMLWoTarget = " \
            <load-file updated=\"1518198961\">\n\
            <resource name=\"mr1_prod\">\n\
            <host name=\"mr1.hm.com\">\n\
            <load>485</load>\n\
            </host>\n\
            </resource>\n\
            </load-file>\n";
    string testXMLWoMax = " \
            <load-file updated=\"1518198961\">\n\
            <resource name=\"mr1_prod\">\n\
            <host name=\"mr1.hm.com\">\n\
            <load>485</load>\n\
            <target>500</target>\n\
            </host>\n\
            </resource>\n\
            </load-file>\n";
    string testXMLWoTime = " \
            <load-file>\n\
            <resource name=\"mr1_prod\">\n\
            <host name=\"mr1.hm.com\">\n\
            <load>485</load>\n\
            <target>500</target>\n\
            <max>5000</max>\n\
            </host>\n\
            </resource>\n\
            </load-file>\n";


    HMAuxCache auxCache;

    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoTime, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoLoad, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoHostNameAttr, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoResources, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoResourceNameAttr, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoHostName, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoMax, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoTarget, HM_AUX_DATA_XML));
}

void TESTNAME::test_HMAuxInfo_OOB()
{

    string testXML = " \
    <oob-file updated=\"1518170180\">\n\
    <resource-oob name=\"test-oob-resource\">\n\
    <host name=\"prod-01.hm.com\">\n\
    <time>1506496234</time>\n\
    <force-down>true</force-down>\n\
    <shed>10</shed>\n\
    </host>\n\
    </resource-oob>\n\
    <resource-oob name=\"my-test-resource\">\n\
    <host name=\"prod-01.hm3.com\">\n\
    <time>1508927205</time>\n\
    <force-down>false</force-down>\n\
    </host>\n\
    </resource-oob>\n\
    <resource-oob name=\"test-oob2-resource\">\n\
    <host name=\"prod-01.hm3.com\">\n\
    <time>1506400318</time>\n\
    <force-down>true</force-down>\n\
    <shed>60</shed>\n\
    </host>\n\
    <host name=\"prod-01.hm.com\">\n\
    <time>1506400318</time>\n\
    <force-down>false</force-down>\n\
    <shed>20</shed>\n\
    </host>\n\
    </resource-oob>\n\
    </oob-file>\n";
    HMAuxInfo auxInfo;
    HMAuxCache auxCache;
    std::vector<std::unique_ptr<HMAuxBase>> singleResult;
    HMAuxOOB* lfb;
    HMTimeStamp ts,ts1,ts2;
    ts.setTime(1506496234);
    ts1.setTime(1508927205);
    ts2.setTime(1506400318);
    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;

    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXML, HM_AUX_DATA_XML));

    CPPUNIT_ASSERT(
            auxCache.getAuxInfo(hostName, sourceURL, address, auxInfo));
    CPPUNIT_ASSERT_EQUAL(4, (int)auxInfo.m_auxData.size());
    lfb = dynamic_cast<HMAuxOOB*>(auxInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(
            lfb->m_host == "prod-01.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "test-oob-resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_shed == 10);
    CPPUNIT_ASSERT(lfb->m_forceDown == true);

    lfb = dynamic_cast<HMAuxOOB*>(auxInfo.m_auxData[1].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    //CPPUNIT_ASSERT(lfb->m_host == "prod-01.hm3.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "my-test-resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts1);
    CPPUNIT_ASSERT_EQUAL(0, (int)lfb->m_shed);
    CPPUNIT_ASSERT(lfb->m_forceDown == false);

    lfb = dynamic_cast<HMAuxOOB*>(auxInfo.m_auxData[2].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    //CPPUNIT_ASSERT(lfb->m_host == "prod-01.hm3.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "test-oob2-resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts2);
    CPPUNIT_ASSERT(lfb->m_shed == 60);
    CPPUNIT_ASSERT(lfb->m_forceDown == true);

    lfb = dynamic_cast<HMAuxOOB*>(auxInfo.m_auxData[3].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    //CPPUNIT_ASSERT(lfb->m_host == "prod-01.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address);
    CPPUNIT_ASSERT(lfb->m_resource == "test-oob2-resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts2);
    CPPUNIT_ASSERT_EQUAL(20, (int)lfb->m_shed);
    CPPUNIT_ASSERT(lfb->m_forceDown == false);

}


void TESTNAME::test_parse_HMAuxInfo_OOB()
{

    string testXMLWoResources = " \
            <oob-file updated=\"1518170180\">\n\
            </oob-file>\n";

    string testXMLWoResourceNameAttr = " \
            <oob-file updated=\"1518170180\">\n\
            <resource-oob>\n\
            </resource-oob>\n\
            </oob-file>\n";

    string testXMLWoHostName = " \
            <oob-file updated=\"1518170180\">\n\
            <resource-oob name=\"test-oob-resource\">\n\
            </resource-oob>\n\
            </oob-file>\n";

    string testXMLWoHostNameAttr = " \
            <oob-file updated=\"1518170180\">\n\
            <resource-oob name=\"test-oob-resource\">\n\
            <host>\n\
            </host>\n\
            </resource-oob>\n\
            </oob-file>\n";

    string testXMLWoTime = " \
            <oob-file>\n\
            <resource-oob name=\"test-oob-resource\">\n\
            <host name=\"prod-01.hm.com\">\n\
            </host>\n\
            </resource-oob>\n\
            </oob-file>\n";

    string testXMLWofile = " \
        <oobI-file updated=\"1518170180\">\n\
        </oobI-file>\n";

    string testWomaxXML = " \
    <load-file updated=\"1518198961\">\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <max>ABC</max>\n\
    </host>\n\
    </resource>\n\
    </load-file>\n";
    
    string testWoloadXML = " \
    <load-file updated=\"1518198961\">\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <load>abc</load>\n\
    </host>\n\
    </resource>\n\
    </load-file>\n";
    
    string testWotargetXML = " \
    <load-file updated=\"1518198961\">\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <target>abc</target>\n\
    </host>\n\
    </resource>\n\
    </load-file>\n";

    string testWoshedXML = " \
    <oob-file updated=\"1518170180\">\n\
    <resource-oob name=\"test-oob-resource\">\n\
    <host name=\"prod-01.hm.com\">\n\
    <time>1506496234</time>\n\
    <force-down>true</force-down>\n\
    <shed>1000</shed>\n\
    </host>\n\
    </resource-oob>\n\
    </oob-file>\n";

    string testWonameXML = " \
    <oob-file updated=\"1518170180\">\n\
    <resource-oob name=\"test-oob-resource\">\n\
    <host>\n\
    <time>1506496234</time>\n\
    <force-down>true</force-down>\n\
    <shed>1000</shed>\n\
    </host>\n\
    </resource-oob>\n\
    </oob-file>\n";


    HMAuxCache auxCache;
    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoTime, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoHostNameAttr, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoResources, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoResourceNameAttr, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWoHostName, HM_AUX_DATA_XML));
    
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWofile, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testWomaxXML, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testWoloadXML, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testWotargetXML, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testXMLWofile, HM_AUX_DATA_XML));

    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testWoshedXML, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(!auxCache.storeAuxInfo(hostName, sourceURL, address, testWonameXML, HM_AUX_DATA_XML));
    
}

void TESTNAME::test_neg_HMAuxInfo_OOB()
{

    string testXML = " \
    <oob-file updated=\"1518170180\">\n\
    <resource-oob name=\"test-oob-resource\">\n\
    <host name=\"prod-01.hm.com\">\n\
    <time>1506496234</time>\n\
    <force-down>true</force-down>\n\
    <shed>10</shed>\n\
    </host>\n\
    </resource-oob>\n\
    <resource-oob name=\"test-oob-resource\">\n\
    <host name=\"prod-01.hm.com\">\n\
    <time>1508927205</time>\n\
    <force-down>false</force-down>\n\
    </host>\n\
    </resource-oob>\n\
    </oob-file>\n";
    HMAuxInfo auxInfo;
    HMAuxCache auxCache;
    std::vector<std::unique_ptr<HMAuxBase>> results;
    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;

    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXML, HM_AUX_DATA_XML));
    CPPUNIT_ASSERT(auxCache.getAuxInfo(hostName, sourceURL, address, auxInfo));

    CPPUNIT_ASSERT(
            auxCache.genAuxData(auxInfo, HM_OOB_FILE, "test.hm.com",
                    xmlOut, HM_AUX_DATA_XML));
}

void TESTNAME::test_neg_HMAuxInfo_NewLFB()
{
    string testXML = " \
    <load-file updated=\"1518198961\">\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <time>1518198900</time>\n\
    <load>485</load>\n\
    <target>2500</target>\n\
    <max>5000</max>\n\
    </host>\n\
    </resource>\n\
    <resource name=\"mr1_prod\">\n\
    <host name=\"mr1.hm.com\">\n\
    <time>1436935860</time>\n\
    <load>900</load>\n\
    <target>500</target>\n\
    <max>1000</max>\n\
    </host>\n\
    </resource>\n\
    </load-file>\n";
    HMAuxInfo auxInfo;
    HMAuxCache auxCache;
    std::vector<std::unique_ptr<HMAuxBase>> results;

    string hostName = "test.hm.com";
    string sourceURL = "lfb.html";
    HMIPAddress address;
    address.set("192.168.2.1");
    string xmlOut;

    CPPUNIT_ASSERT(auxCache.storeAuxInfo(hostName, sourceURL, address, testXML, HM_AUX_DATA_XML));

    CPPUNIT_ASSERT(auxCache.getAuxInfo(hostName, sourceURL, address, auxInfo));

    CPPUNIT_ASSERT(
            auxCache.genAuxData(auxInfo, HM_LOAD_FILE, "test.hm.com",
                    xmlOut, HM_AUX_DATA_XML));
}
