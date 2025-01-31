/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessage.hpp"
#include "NetworkBrokerData.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "gmlc/containers/BlockingPriorityQueue.hpp"
#include <functional>
#include <thread>

namespace helics
{
enum class interface_networks : char;

/** implementation of a generic communications interface
 */
class CommsInterface
{
  public:
    /** enumeration of whether the threading system should generate a single thread or multiple threads*/
    enum class thread_generation
    {
        single,  // indicate that a single thread is used for transmitting and receiving
        dual  // indicate that separate threads are used 1 for transmission and one for reception
    };
    /** default constructor*/
    CommsInterface () = default;
    explicit CommsInterface (thread_generation threads);
    /** destructor*/
    virtual ~CommsInterface ();

    /** load network information into the comms object*/
    virtual void loadNetworkInfo (const NetworkBrokerData &netInfo);
    void loadTargetInfo (const std::string &localTarget,
                         const std::string &brokerTarget,
                         interface_networks targetNetwork = interface_networks::local);
    /** transmit a message along a particular route
     */
    void transmit (route_id rid, const ActionMessage &cmd);
    /** transmit a message along a particular route
     */
    void transmit (route_id rid, ActionMessage &&cmd);
    /** add a new route assigned to the appropriate id
     */
    void addRoute (route_id rid, const std::string &routeInfo);
    /** remove a route from use*/
    void removeRoute (route_id rid);
    /** connect the commsInterface
    @return true if the connection was successful false otherwise
    */
    bool connect ();

    /** disconnected the comms interface
     */
    void disconnect ();

    /** try reconnected from a mismatched or disconnection*/
    bool reconnect ();
    /** set the name of the communicator*/
    void setName (const std::string &name);
    /** set the callback for processing the messages
     */
    void setCallback (std::function<void (ActionMessage &&)> callback);
    /** set the callback for processing the messages
     */
    void setLoggingCallback (
      std::function<void (int level, const std::string &name, const std::string &message)> callback);
    /** set the max message size and max Queue size
     */
    void setMessageSize (int maxMsgSize, int maxCount);
    /** check if the commInterface is connected
     */
    bool isConnected () const;

    /** set the timeout for the initial broker connection
    @param timeOut the value is in milliseconds
    */
    void setTimeout (std::chrono::milliseconds timeOut);
    /** set a flag for the comms system*/
    virtual void setFlag (const std::string &flag, bool val);
    /** enable or disable the server mode for the comms*/
    void setServerMode (bool serverActive);

    /** generate a log message as a warning*/
    void logWarning (const std::string &message) const;
    /** generate a log message as an error*/
    void logError (const std::string &message) const;
    /** generate a log message as a level above warning or error*/
    void logMessage (const std::string &message) const;

  protected:
    // enumeration of the connection status flags for more immediate feedback from the processing threads
    enum class connection_status : int
    {

        startup = -1,  //!< the connection is in startup mode
        connected = 0,  //!< we are connected
        reconnecting = 1,  //!< we are trying reconnect
        terminated = 2,  //!< the connection has been terminated
        error = 4,  //!< some error occurred on the connection

    };

  private:
    std::atomic<connection_status> rx_status{connection_status::startup};  //!< the status of the receiver thread
  protected:
    gmlc::concurrency::TriggerVariable rxTrigger;

    std::string name;  //!< the name of the object
    std::string localTargetAddress;  //!< the base for the receive address
    std::string brokerTargetAddress;  //!< the base for the broker address
    std::string brokerName;  //!< the identifier for the broker
    std::string brokerInitString;  //!< the initialization string for any automatically generated broker
  private:
    std::atomic<connection_status> tx_status{
      connection_status::startup};  //!< the status of the transmitter thread
    gmlc::concurrency::TriggerVariable txTrigger;
    std::atomic<bool> operating{false};  //!< the comms interface is in startup mode
    const bool singleThread{false};

  protected:
    bool serverMode = true;  //!< some comms have a server mode and non-server mode
    bool autoBroker = false;  //!< the broker should be automatically generated if needed
    std::chrono::milliseconds connectionTimeout{
      4000};  // timeout for the initial connection to a broker or to bind a broker port(in ms)
    int maxMessageSize = 16 * 1024;  //!< the maximum message size for the queues (if needed)
    int maxMessageCount = 512;  //!< the maximum number of message to buffer (if needed)
    std::atomic<bool> requestDisconnect{false};  //!< flag gets set when disconnect is called
    std::function<void (ActionMessage &&)>
      ActionCallback;  //!< the callback for what to do with a received message
    std::function<void (int level, const std::string &name, const std::string &message)>
      loggingCallback;  //!< callback for logging
    gmlc::containers::BlockingPriorityQueue<std::pair<route_id, ActionMessage>>
      txQueue;  //!< set of messages waiting to be transmitted
    // closing the files or connection can take some time so there is a need for inter-thread communication to not
    // spit out warning messages if it is in the process of disconnecting
    std::atomic<bool> disconnecting{
      false};  //!< flag indicating that the comm system is in the process of disconnecting
    interface_networks interfaceNetwork = interface_networks::local;

  private:
    std::thread queue_transmitter;  //!< single thread for sending data
    std::thread queue_watcher;  //!< thread monitoring the receive queue
    std::mutex threadSyncLock;  //!< lock to handle thread operations
    virtual void queue_rx_function () = 0;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () = 0;  //!< the loop for transmitting data
    virtual void closeTransmitter ();  //!< function to instruct the transmitter loop to close
    virtual void closeReceiver ();  //!< function to instruct the receiver loop to close
    virtual void reconnectTransmitter ();  //!< function to reconnect the transmitter
    virtual void reconnectReceiver ();  //!< function to reconnect the receiver
  protected:
    void setTxStatus (connection_status txStatus);
    void setRxStatus (connection_status rxStatus);
    connection_status getRxStatus () const { return rx_status.load (); }
    connection_status getTxStatus () const { return tx_status.load (); }
    /** function to protect certain properties in a threaded environment
    these functions should be called in a pair*/
    bool propertyLock ();
    void propertyUnLock ();

  private:
    gmlc::concurrency::TripWireDetector tripDetector;  //!< try to detect if everything is shutting down
};

template <class X>
class ConditionalChangeOnDestroy
{
  private:
    std::atomic<X> &aref;
    X fval;
    X expectedValue;

  public:
    ConditionalChangeOnDestroy (std::atomic<X> &var, X finalValue, X expValue)
        : aref (var), fval (std::move (finalValue)), expectedValue (std::move (expValue))
    {
    }
    ~ConditionalChangeOnDestroy () { aref.compare_exchange_strong (expectedValue, fval); }
};

}  // namespace helics
