/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_CORE_HPP_
#define HELICS_CPP98_CORE_HPP_
#pragma once

#include "../cpp98/Filter.hpp"
#include "../shared_api_library/MessageFilters.h"
#include "../shared_api_library/helics.h"
#include "config.hpp"
#include <stdexcept>
#include <string>

namespace helicscpp
{
class Core
{
  public:
    /** Default constructor*/
    Core () HELICS_NOTHROW : core (HELICS_NULL_POINTER){};
    /** construct with type, core name and initialization string */
    Core (const std::string &type, const std::string &name, const std::string &initString)
    {
        core = helicsCreateCore (type.c_str (), name.c_str (), initString.c_str (), hThrowOnError ());
    }
    /** construct with type, core name and command line arguments */
    Core (const std::string &type, const std::string &name, int argc, char **argv)
    {
        core = helicsCreateCoreFromArgs (type.c_str (), name.c_str (), argc, argv, hThrowOnError ());
    }
    /** destructor*/
    ~Core () { helicsCoreFree (core); }
    /** implicit operator so the object can be used with the c api functions natively*/
    operator helics_core () { return core; }
    /** explicitly get the base helics_core object*/
    helics_core baseObject () const { return core; }
    /** check if the core is connected to the broker*/
    bool isConnected () const { return helicsCoreIsConnected (core); }
    /** copy constructor*/
    Core (const Core &cr) { core = helicsCoreClone (cr.core, hThrowOnError ()); }
    /** copy assignment*/
	Core &operator= (const Core &cr)
    {
        core = helicsCoreClone (cr.core, hThrowOnError ());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor*/
    Core (Core &&cr) HELICS_NOTHROW
    {
        core = cr.core;
        cr.core = HELICS_NULL_POINTER;
    }
    /** move assignment*/
    Core &operator= (Core &&cr) HELICS_NOTHROW
    {
        core = cr.core;
        cr.core = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    /** set the core to ready to enter init
   @details this function only needs to be called for cores that don't have any federates but may
   have filters for cores with federates it won't do anything*/
    void setReadyToInit () { helicsCoreSetReadyToInit (core, hThrowOnError ()); }
    
	/**
     * disconnect the core from its broker
     */
	void disconnect () { helicsCoreDisconnect (core, hThrowOnError ()); }
    /** waits in the current thread until the broker is disconnected
    @param msToWait  the timeout to wait for disconnect (-1) implies no timeout
    @return true if the disconnect was successful false if it timed out
     */
    bool waitForDisconnect (int msToWait = -1)
    {
        return helicsCoreWaitForDisconnect (core, msToWait, hThrowOnError ());
    }
    /** get an identifier string for the core
     */
    const char *getIdentifier () const { return helicsCoreGetIdentifier (core); }
    /** get the connection network or connection address for the core*/
    const char *getAddress () const { return helicsCoreGetAddress (core); }
    /** create a destination Filter on the specified federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param type the type of filter to create
    @param name the name of the filter (can be NULL)
    @return a helics_filter object
    */
    Filter registerFilter (helics_filter_type type, const std::string &name = std::string ())
    {
        return Filter (helicsCoreRegisterFilter (core, type, name.c_str (), hThrowOnError ()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a helics_filter object
    */
    CloningFilter registerCloningFilter (const std::string &deliveryEndpoint)
    {
        return CloningFilter (helicsCoreRegisterCloningFilter (core, deliveryEndpoint.c_str (), hThrowOnError ()));
    }

    /** set a global federation value
    @param valueName the name of the global value to set
    @param value actual value of the global variable
    */
    void setGlobal (const std::string &valueName, const std::string &value)
    {
        helicsCoreSetGlobal (core, valueName.c_str (), value.c_str (), hThrowOnError ());
    }

  protected:
    helics_core core;  //!< reference to the underlying core object
};

}  // namespace helicscpp
#endif
