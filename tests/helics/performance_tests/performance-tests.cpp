/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

//#include <iostream>
#include "helics/helics-config.h"
#include <helics/application_api/Federate.hpp>

#ifndef BOOST_STATIC
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE performance_tests
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include <boost/test/unit_test.hpp>

struct globalTestConfig
{
    globalTestConfig () = default;
    ~globalTestConfig ()
    {
        // std::cout << "cleaning up" << std::endl;
        helics::cleanupHelicsLibrary ();
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);
