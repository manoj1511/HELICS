/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_VALUE_FEDERATE_HPP_
#define HELICS_CPP98_VALUE_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "Federate.hpp"
#include "Input.hpp"
#include "Publication.hpp"
#include <exception>
#include <sstream>

namespace helicscpp
{
/** enumeration of the available types of publications and inputs*/
enum PubSubTypes
{
    STRING_TYPE = helics_data_type_string,
    DOUBLE_TYPE = helics_data_type_double,
    INT_TYPE = helics_data_type_int,
    COMPLEX_TYPE = helics_data_type_complex,
    VECTOR_TYPE = helics_data_type_vector,
    TIME_TYPE = helics_data_type_time,
    BOOLEAN_TYPE = helics_data_type_boolean,
    RAW_TYPE = helics_data_type_raw
};

/** Class defining a ValueFederate object which interacts with publication and Inputs*/
class ValueFederate : public virtual Federate
{
  private:
    std::vector<helics_input> ipts;
    std::vector<helics_publication> pubs;

  public:
    friend class helicscpp::FederateInfo;
    /**constructor taking a federate information structure and using the default core
   @param fedName the name of the federate, can be empty to use the name from fi or an auto generated one
   @param fi  a federate information structure
   */
    explicit ValueFederate (const std::string &fedName, FederateInfo &fi)
    {
        fed = helicsCreateValueFederate (fedName.c_str (), fi.getInfo (), hThrowOnError ());
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL"));
        }
    }
    /**constructor taking a string with the required information
    @param configString can be either a JSON file, a TOML file (with extension TOML), or a string containing JSON
    code
    */
    explicit ValueFederate (const std::string &configString)
    {
        fed = helicsCreateValueFederateFromConfig (configString.c_str (), hThrowOnError ());
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL"));
        }
    }
    /** copy constructor*/
    ValueFederate (const ValueFederate &vfed) : Federate (vfed), ipts (vfed.ipts), pubs (vfed.pubs) {}
    /** copy assignment operator*/
	ValueFederate &operator= (const ValueFederate &fedObj)
    {
        Federate::operator= (fedObj);
        ipts = fedObj.ipts;
        pubs = fedObj.pubs;
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL move constructor"));
        }
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor*/
    ValueFederate (ValueFederate &&fedObj) HELICS_NOTHROW : Federate (),
                                                            ipts (std::move (fedObj.ipts)),
                                                            pubs (std::move (fedObj.pubs))
    {
        Federate::operator= (std::move (fedObj));
    }
    /** move assignment operator*/
    ValueFederate &operator= (ValueFederate &&fedObj) HELICS_NOTHROW
    {
        ipts = std::move (fedObj.ipts);
        pubs = std::move (fedObj.pubs);
        Federate::operator= (std::move (fedObj));
        return *this;
    }
#endif
    /** Default constructor, not meant to be used */
    ValueFederate () HELICS_NOTHROW {}

    /** Methods to register publications **/

	/** register a publication
    @details call is only valid in startup mode
    @param name the name of the publication
    @param type a string defining the type of the publication
    @param units a string defining the units of the publication [optional]
    @return a publication id object for use as an identifier
    */
    Publication
    registerPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterTypePublication (fed, name.c_str (), type.c_str (),
                                                                        units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

	/** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param name the name of the publication
	@param type the type of publication to register 
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    Publication registerPublication (const std::string &name, helics_data_type type, const std::string &units = "")
    {
        helics_publication pub =
          helicsFederateRegisterPublication (fed, name.c_str (), type, units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

	/** register a publication
    @details call is only valid in startup mode
    @param name the name of the publication
    @param type a string defining the type of the publication
    @param units a string defining the units of the publication [optional]
    @return a publication object for use as an identifier
    */
    Publication
    registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalTypePublication (fed, name.c_str (), type.c_str (),
                                                                              units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

	/** register a publication
    @details call is only valid in startup mode
    @param key the name of the publication
    @param type an enumeration value describing the type of the publication
    @param units a string defining the units of the publication [optional]
    @return a publication object for use as an identifier
    */
    Publication
    registerGlobalPublication (const std::string &key, helics_data_type type, const std::string &units = "")
    {
        helics_publication pub =
          helicsFederateRegisterGlobalPublication (fed, key.c_str (), type, units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

	/** register a publication as part of an indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the index appended
    @param key the name of the publication to register
    @param index1 an index associated with the publication
	@param type an enumeration value describing the type of the publication
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    Publication registerPublicationIndexed (const std::string &key,
                                            int index1,
                                            helics_data_type type,
                                            const std::string &units = "")
    {
        std::string indexed_name = key + '_' + toStr (index1);
        return registerGlobalPublication (indexed_name, type, units);
    }

	/** register a publication as part of a 2 dimensional indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the indices appended
    @param key the base name of the publication
    @param index1 an index associated with the publication
    @param index2 a second index
    @param type an enumeration value describing the type of the publication
	@param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    Publication registerPublicationIndexed (const std::string &key,
                                            int index1,
                                            int index2,
                                            helics_data_type type,
                                            const std::string &units = std::string ())
    {
        std::string indexed_name = key+ '_' + toStr (index1) + '_' + toStr (index2);
        return registerGlobalPublication (indexed_name, type, units);
    }
    /** register publications   from a JSON output file or string
	@details generates interface based on the data contained in a JSON file
	or string
	*/
    void registerFromPublicationJSON (const std::string &json)
    {
        helicsFederateRegisterFromPublicationJSON (fed, json.c_str (), hThrowOnError ());
    }
    /** get  publication by name*/
    Publication getPublication (const std::string &name)
    {
        return Publication (helicsFederateGetPublication (fed, name.c_str (), hThrowOnError ()));
    }
    /** get a publication by index
	@param index a 0 based index of the publication to retrieve
	@return a Publication object*/
    Publication getPublication (int index)
    {
        return Publication (helicsFederateGetPublicationByIndex (fed, index, hThrowOnError ()));
    }
    /** Methods to register subscriptions **/
    Input registerSubscription (const std::string &name, const std::string &units = std::string ())
    {
        helics_input sub =
          helicsFederateRegisterSubscription (fed, name.c_str (), units.c_str (), hThrowOnError ());
        ipts.push_back (sub);
        return Input (sub);
    }

	/** register a !D indexed subscription
    @param name the base name of the publication to subscribe to
    @param index1 the first index of the value to subscribe to
    @param units a string containing the requested units of the subscription output
    @return an input object getting the requested value
    */
    Input registerSubscriptionIndexed (const std::string &name, int index1, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerSubscription (indexed_name, units);
    }

    /** register a 2D indexed subscription
	@param name the base name of the publication to subscribe to
	@param index1 the first index of the value to subscribe to
	@param index2 the second index of the value to subscribe to
	@param units a string containing the requested units of the subscription output
	@return an input object getting the requested value
	*/
    Input
    registerSubscriptionIndexed (const std::string &name, int index1, int index2, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerSubscription (indexed_name, units);
    }
    /** get an input by index*/
    Input getInput (const std::string &name)
    {
        return Input (helicsFederateGetInput (fed, name.c_str (), hThrowOnError ()));
    }
    /** get a subscription by index*/
    Input getSubscription (int index)
    {
        return Input (helicsFederateGetInputByIndex (fed, index, hThrowOnError ()));
    }
    /** get the number of inputs in this federate*/
    int getInputCount () const { return helicsFederateGetInputCount (fed); }
    /** get the number of publications in this federate*/
    int getPublicationCount () const { return helicsFederateGetPublicationCount (fed); }
    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_input> queryUpdates () { return std::vector<helics_input> (); }

    /** clear all the update flags from all federate inputs*/
    void clearUpdates () { helicsFederateClearUpdates (fed); }
    /** publish data contained in a JSON file*/
    void publishJSON (const std::string &json)
    {
        helicsFederatePublishJSON (fed, json.c_str (), hThrowOnError ());
    }

  private:
    // Utility function for converting numbers to string
    template <typename T>
    std::string toStr (T num)
    {
        std::ostringstream ss;
        ss << num;
        return ss.str ();
    }
};
}  // namespace helicscpp
#endif
