/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/helics-config.h"
#include "helics/flag-definitions.h"

/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.
*/

/** enumeration of defined print levels*/
enum log_level : int
{
    no_print = LOG_LEVEL_NO_PRINT,  //!< never print
    error = LOG_LEVEL_ERROR,  //!< only print errors
    warning = LOG_LEVEL_WARNING,  //!< print/log warning and errors
    summary = LOG_LEVEL_SUMMARY,  //!< print/log summary information
    connections = LOG_LEVEL_CONNECTIONS,  //!< print summary+ federate level connection information
    interfaces = LOG_LEVEL_INTERFACES,  //!< print connections +interface level connection information
    timing = LOG_LEVEL_TIMING,  //!< print interfaces+ timing(exec/grant/disconnect)
    data = LOG_LEVEL_DATA,  //!< print timing+data transmissions
    trace = LOG_LEVEL_TRACE,  //!< trace level printing (all processed messages)
};

#define LOG_ERROR(id, ident, message) sendToLogger (id, log_level::error, ident, message);
#define LOG_WARNING(id, ident, message) sendToLogger (id, log_level::warning, ident, message);

#ifdef ENABLE_LOGGING
#define LOG_SUMMARY(id, ident, message)                                                                           \
    if (maxLogLevel >= log_level::summary)                                                                        \
    {                                                                                                             \
        sendToLogger (id, log_level::summary, ident, message);                                                    \
    }

#define LOG_CONNECTIONS(id, ident, message)                                                                       \
    if (maxLogLevel >= log_level::connections)                                                                    \
    {                                                                                                             \
        sendToLogger (id, log_level::connections, ident, message);                                                \
    }

#define LOG_INTERFACES(id, ident, message)                                                                        \
    if (maxLogLevel >= log_level::interfaces)                                                                     \
    {                                                                                                             \
        sendToLogger (id, log_level::interfaces, ident, message);                                                 \
    }

#ifdef ENABLE_DEBUG_LOGGING
#define LOG_TIMING(id, ident, message)                                                                            \
    if (maxLogLevel >= log_level::timing)                                                                         \
    {                                                                                                             \
        sendToLogger (id, log_level::timing, ident, message);                                                     \
    }
#define LOG_DATA_MESSAGES(id, ident, message)                                                                     \
    if (maxLogLevel >= log_level::data)                                                                           \
    {                                                                                                             \
        sendToLogger (id, log_level::data, ident, message);                                                       \
    }
#else
#define LOG_TIMING(id, ident, message)
#define LOG_DATA_MESSAGES(id, ident, message)
#endif

#ifdef ENABLE_TRACE_LOGGING
#define LOG_TRACE(id, ident, message)                                                                             \
    if (maxLogLevel >= log_level::trace)                                                                          \
    {                                                                                                             \
        sendToLogger (id, log_level::trace, ident, message);                                                      \
    }
#else
#define LOG_TRACE(id, ident, message)
#endif
#else
#define LOG_SUMMARY(id, ident, message)
#define LOG_CONNECTIONS(id, ident, message)
#define LOG_INTERFACES(id, ident, message)
#define LOG_TIMING(id, ident, message)
#define LOG_DATA_MESSAGES(id, ident, message)
#define LOG_TRACE(id, ident, message)
#endif
