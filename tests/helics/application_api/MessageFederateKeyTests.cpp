/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "testFixtures.hpp"

#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (message_federate_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/

BOOST_DATA_TEST_CASE (message_federate_send_receive, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint ("ep2", "random");
    mFed1->setProperty (helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    helics::data_block data (500, 'a');

    mFed1->sendMessage (epid, "ep2", data);

    auto time = mFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = mFed1->hasMessage (epid);
    BOOST_CHECK (res == false);
    res = mFed1->hasMessage (epid2);
    BOOST_CHECK (res);

    auto M = mFed1->getMessage (epid2);
    BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M->data[245], data[245]);
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_obj, bdata::make (core_types_single), core_type)
{
    using namespace helics;
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    Endpoint epid (mFed1.get (), "ep1");

    Endpoint epid2 (GLOBAL, mFed1.get (), "ep2", "random");
    mFed1->setProperty (helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    helics::data_block data (500, 'a');

    epid.send ("ep2", data);

    auto time = mFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = epid.hasMessage ();
    BOOST_CHECK (res == false);
    res = epid2.hasMessage ();
    BOOST_CHECK (res);

    auto M = epid2.getMessage ();
    BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M->data[245], data[245]);
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed, bdata::make (core_types), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");

    mFed1->setProperty (helics_property_time_delta, 1.0);
    mFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    mFed1->sendMessage (epid, "ep2", data);
    mFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = mFed1->hasMessage (epid);
    BOOST_CHECK (res);
    res = mFed2->hasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = mFed1->getMessage (epid);
    BOOST_REQUIRE (M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage (epid2);
    BOOST_REQUIRE (M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);
    mFed1->finalize ();
    mFed2->finalize ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_AUTO_TEST_CASE (message_federate_send_receive_2fed_extra)
{
    SetupTest<helics::MessageFederate> ("test_7", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);
    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");

    mFed1->setProperty (helics_property_time_delta, 1.0);
    mFed2->setProperty (helics_property_time_delta, 1.0);
    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();
    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    mFed1->sendMessage (epid, "ep2", data);
    mFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = mFed1->hasMessage (epid);
    BOOST_CHECK (res);
    res = mFed2->hasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = mFed1->getMessage (epid);
    BOOST_REQUIRE (M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage (epid2);
    BOOST_REQUIRE (M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);
    mFed1->finalizeAsync ();
    mFed2->finalize ();
    mFed1->finalizeComplete ();
    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_obj, bdata::make (core_types), core_type)
{
    using namespace helics;
    SetupTest<MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<MessageFederate> (0);
    auto mFed2 = GetFederateAs<MessageFederate> (1);

    Endpoint epid (mFed1, "ep1");

    Endpoint epid2 (GLOBAL, mFed2.get (), "ep2", "random");

    mFed1->setProperty (helics_property_time_delta, 1.0);
    mFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    epid.send ("ep2", data);
    epid2.send ("fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = epid.hasMessage ();
    BOOST_CHECK (res);
    epid2.hasMessage ();
    BOOST_CHECK (res);

    auto M1 = epid.getMessage ();
    BOOST_REQUIRE (M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data2[245]);

    auto M2 = epid2.getMessage ();
    BOOST_REQUIRE (M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);
    mFed1->finalizeAsync ();
    mFed2->finalize ();
    mFed1->finalizeComplete ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_multisend, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty (helics_property_time_delta, 1.0);
    mFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data1 (500, 'a');
    helics::data_block data2 (400, 'b');
    helics::data_block data3 (300, 'c');
    helics::data_block data4 (200, 'd');
    epid.setDefaultDestination ("ep2");
    mFed1->sendMessage (epid, "ep2", data1);
    mFed1->sendMessage (epid, "ep2", data2);
    epid.send (data3);
    epid.send (data4);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    BOOST_CHECK (!mFed1->hasMessage ());

    BOOST_CHECK (!mFed1->hasMessage (epid));
    auto cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 4);

    BOOST_CHECK_EQUAL (epid.getDefaultDestination (), "ep2");
    auto M1 = mFed2->getMessage (epid2);
    BOOST_REQUIRE (M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data1.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 3);
    auto M2 = mFed2->getMessage ();
    BOOST_REQUIRE (M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data2.size ());
    BOOST_CHECK_EQUAL (M2->data[245], data2[245]);
    cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 2);

    auto M3 = mFed2->getMessage ();
    auto M4 = mFed2->getMessage (epid2);
    BOOST_REQUIRE (M3);
    BOOST_REQUIRE (M4);
    BOOST_CHECK_EQUAL (M3->data.size (), data3.size ());
    BOOST_CHECK_EQUAL (M4->data.size (), data4.size ());

    BOOST_CHECK_EQUAL (M4->source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4->dest, "ep2");
    BOOST_CHECK_EQUAL (M4->original_source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4->time, 0.0);
    mFed1->finalizeAsync ();
    mFed2->finalize ();
    mFed1->finalizeComplete ();
    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (test_time_interruptions, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");
    mFed1->setProperty (helics_property_time_delta, 1);
    mFed2->setProperty (helics_property_time_delta, 0.5);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    mFed1->sendMessage (epid, "ep2", data);
    mFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_REQUIRE_EQUAL (gtime, 0.5);

    BOOST_REQUIRE (mFed2->hasMessage (epid2));

    auto M2 = mFed2->getMessage (epid2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);

    gtime = mFed2->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    auto M1 = mFed1->getMessage (epid);
    BOOST_CHECK (M1);
    if (M1)
    {
        BOOST_CHECK_EQUAL (M1->data.size (), data2.size ());
        if (M1->data.size () > 245)
        {
            BOOST_CHECK_EQUAL (M1->data[245], data2[245]);
        }
    }

    BOOST_CHECK (mFed1->hasMessage () == false);
    mFed1->finalizeAsync ();

    gtime = mFed2->requestTime (2.0);
    BOOST_CHECK_EQUAL (gtime, 2.0);
    mFed2->finalize ();
    mFed1->finalizeComplete ();
    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_AUTO_TEST_SUITE_END ()
