/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "HelicsPrimaryTypes.hpp"
#include "ValueFederate.hpp"
#include "helicsTypes.hpp"
#include <algorithm>
#include <array>
namespace helics
{
/** base class for a input object*/
class Input
{
  protected:
    ValueFederate *fed = nullptr;  //!< reference to the value federate
    interface_handle handle;  //!< the id as generated by the Federate
  private:
    int referenceIndex = -1;  //!< an index used for callback lookup
    void *dataReference = nullptr;  //!< pointer to a piece of containing data

    helics_type_t type = helics_type_t::helicsUnknown;  //!< the underlying type the publication is using
    bool changeDetectionEnabled = false;  //!< the change detection is enabled
    bool hasUpdate = false;  //!< the value has been updated
    size_t customTypeHash = 0;  //!< a hash code for the custom type
    defV lastValue;  //!< the last value updated
    double delta = -1.0;  //!< the minimum difference
    std::string actualName;  //!< the name of the federate
    // this needs to match the defV type
    mpark::variant<std::function<void(const double &, Time)>,
                   std::function<void(const int64_t &, Time)>,
                   std::function<void(const std::string &, Time)>,
                   std::function<void(const std::complex<double> &, Time)>,
                   std::function<void(const std::vector<double> &, Time)>,
                   std::function<void(const std::vector<std::complex<double>> &, Time)>,
                   std::function<void(const named_point &, Time)>,
                   std::function<void(const bool &, Time)>,
                   std::function<void(const Time &, Time)>>
      value_callback;  //!< callback function for the federate
  public:
    Input () = default;

    Input (ValueFederate *valueFed, interface_handle id, const std::string &actName)
        : fed (valueFed), handle (id), actualName (actName)
    {
    }

    Input (ValueFederate *valueFed,
           const std::string &name,
           const std::string &defaultType = "def",
           const std::string &units = std::string ());

    template <class FedPtr>
    Input (FedPtr &valueFed,
           const std::string &name,
           const std::string &defaultType = "def",
           const std::string &units = std::string ())
        : Input (std::addressof (*valueFed), name, defaultType, units)
    {
        static_assert (std::is_base_of<ValueFederate, std::remove_reference_t<decltype (*valueFed)>>::value,
                       "first argument must be a pointer to a ValueFederate");
    }

    Input (interface_visibility locality,
           ValueFederate *valueFed,
           const std::string &name,
           const std::string &defaultType = "def",
           const std::string &units = std::string ());

    template <class FedPtr>
    Input (interface_visibility locality,
           FedPtr &valueFed,
           const std::string &name,
           const std::string &defaultType = "def",
           const std::string &units = std::string ())
        : Input (locality, std::addressof (*valueFed), name, defaultType, units)
    {
        static_assert (std::is_base_of<ValueFederate, std::remove_reference_t<decltype (*valueFed)>>::value,
                       "first argument must be a pointer to a ValueFederate");
    }

    Input (ValueFederate *valueFed,
           const std::string &name,
           helics_type_t defType,
           const std::string &units = std::string ())
        : Input (valueFed, name, typeNameStringRef (defType), units)
    {
    }

    template <class FedPtr>
    Input (FedPtr &valueFed,
           const std::string &name,
           helics_type_t defType,
           const std::string &units = std::string ())
        : Input (valueFed, name, typeNameStringRef (defType), units)
    {
    }

    Input (interface_visibility locality,
           ValueFederate *valueFed,
           const std::string &name,
           const std::string &units = std::string ())
        : Input (locality, valueFed, name, "def", units)
    {
    }

    template <class FedPtr>
    Input (interface_visibility locality,
           FedPtr &valueFed,
           const std::string &key,
           const std::string &units = std::string ())
        : Input (locality, valueFed, key, "def", units)
    {
    }

    Input (interface_visibility locality,
           ValueFederate *valueFed,
           const std::string &name,
           helics_type_t defType,
           const std::string &units = std::string ())
        : Input (locality, valueFed, name, typeNameStringRef (defType), units)
    {
    }

    template <class FedPtr>
    Input (interface_visibility locality,
           FedPtr &valueFed,
           const std::string &name,
           helics_type_t defType,
           const std::string &units = std::string ())
        : Input (locality, valueFed, name, typeNameStringRef (defType), units)
    {
    }

    /** get the time of the last update
    @return the time of the last update
    */
    Time getLastUpdate () const { return fed->getLastUpdateTime (*this); }
    /** get the underlying handle that can be used to make direct calls to the Core API
     */
    interface_handle getHandle () const { return handle; }

    /** check if the Publication links to a valid operation*/
    bool isValid () const { return handle.isValid (); }
    bool operator< (const Input &inp) const { return (handle < inp.handle); }
    /** register a callback for an update notification
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void( Time time)
    time is the time the value was updated  This callback is a notification callback and doesn't return the value
    */
    void registerNotificationCallback (std::function<void(Time)> callback)
    {
        fed->setInputNotificationCallback (*this, [this, callback](const Input &, Time time) {
            if (isUpdated ())
            {
                callback (time);
            }
        });
    }
    /** get the Name for the subscription
    @details the name is the local name if given, key is the full key name*/
    const std::string &getName () const { return actualName; }
    /** get the type of the input*/
    const std::string &getType () const { return fed->getInputType (*this); }
    /** get the units associated with a input*/
    const std::string &getUnits () const { return fed->getInputUnits (*this); }
    /** get an associated target*/
    const std::string &getTarget () const { return fed->getTarget (*this); }
    void addTarget (const std::string &newTarget) { fed->addTarget (*this, newTarget); }

    /** check if the value has been updated
    @details if changeDetection is Enabled this function also loads the value into the buffer*/
    bool isUpdated ();

    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    template <class X>
    void setInputNotificationCallback (std::function<void(const X &, Time)> callback)
    {
        static_assert (
          helicsType<X> () != helics_type_t::helicsCustom,
          "callback type must be a primary helics type one of \"double, int64_t, named_point, bool, Time "
          "std::vector<double>, std::vector<std::complex<double>>, std::complex<double>\"");
        value_callback = std::move (callback);
        fed->setInputNotificationCallback (*this, [this](Input &, Time time) { handleCallback (time); });
    }

  private:
    template <class X>
    void setDefault_impl (std::integral_constant<int, 0> /*V*/, X &&val)
    {
        /** still need to make_valid for bool and Time*/
        lastValue = make_valid (std::forward<X> (val));
    }

    template <class X>
    void setDefault_impl (std::integral_constant<int, 1> /*V*/, X &&val)
    {
        lastValue = make_valid (std::forward<X> (val));
    }

    template <class X>
    void setDefault_impl (std::integral_constant<int, 2> /*V*/, X &&val)
    {
        fed->setDefaultValue (*this, ValueConverter<remove_cv_ref<X>>::convert (std::forward<X> (val)));
    }

  public:
    /** set the default value to use before any update has been published
     */
    template <class X>
    void setDefault (X &&val)
    {
        setDefault_impl<X> (typeCategory<X> (), std::forward<X> (val));
    }

    /** set the minimum delta for change detection
    @param detltaV a double with the change in a value in order to register a different value
    */
    void setMinimumChange (double deltaV)
    {
        // this first check enables change detection if it was disabled via negative delta
        if (delta < 0.0)
        {
            changeDetectionEnabled = true;
        }
        delta = deltaV;
        // the second checks if we should disable from negative delta
        if (delta < 0.0)
        {
            changeDetectionEnabled = false;
        }
    }
    /** enable change detection
    @param enabled (optional) set to false to disable change detection true(default) to enable it
    */
    void enableChangeDetection (bool enabled = true) { changeDetectionEnabled = enabled; }

  private:
    /** deal with the callback from the application API*/
    void handleCallback (Time time);
    template <class X>
    void getValue_impl (std::integral_constant<int, 0> /*V*/, X &out);

    /** handle special case for character return data*/
    void getValue_impl (std::integral_constant<int, 0> /*V*/, char &out) { out = getValueChar (); }

    /** handle special case for character return data*/
    void getValue_impl (std::integral_constant<int, 1> /*V*/, char &out) { out = getValueChar (); }

    template <class X>
    void getValue_impl (std::integral_constant<int, 1> /*V*/, X &out)
    {
        std::conditional_t<std::is_integral<X>::value,
                           std::conditional_t<std::is_same<X, char>::value, char, int64_t>, double>
          gval;
        getValue_impl (std::integral_constant<int, 0> (), gval);
        out = static_cast<X> (gval);
    }

    template <class X>
    void getValue_impl (std::integral_constant<int, 2> /*V*/, X &out)
    {
        ValueConverter<X>::interpret (fed->getValueRaw (*this), out);
    }

    template <class X>
    X getValue_impl (std::integral_constant<int, 0> /*V*/)
    {
        X val;
        getValue_impl (std::integral_constant<int, 0> (), val);
        return val;
    }

    template <class X>
    X getValue_impl (std::integral_constant<int, 1> /*V*/)
    {
        std::conditional_t<std::is_integral<X>::value,
                           std::conditional_t<std::is_same<X, char>::value, char, int64_t>, double>
          gval;
        getValue_impl (std::integral_constant<int, 0> (), gval);
        return static_cast<X> (gval);
    }

    template <class X>
    X getValue_impl (std::integral_constant<int, 2> /*V*/)
    {
        return ValueConverter<X>::interpret (fed->getValueRaw (*this));
    }

  public:
    /** get double vector value functions to retrieve data by a C array of doubles*/
    int getValue (double *data, int maxsize);
    /** get string value functions to retrieve data by a C string*/
    int getValue (char *str, int maxsize);
    /** get the latest value for the subscription
    @param[out] out the location to store the value
    */
    template <class X>
    void getValue (X &out)
    {
        getValue_impl<X> (typeCategory<X> (), out);
    }
    /** get the most recent value
    @return the value*/
    template <class X>
    auto getValue ()
    {
        return getValue_impl<remove_cv_ref<X>> (typeCategory<X> ());
    }

    template <class X>
    const X &getValueRef ();

    /** get the raw binary data*/
    data_view getRawValue ();
    /** get the size of the raw data*/
    size_t getRawSize ();
    /** get the size of the data if it were a string*/
    size_t getStringSize ();
    /** get the number of elements in the data if it were a vector*/
    size_t getVectorSize ();

  private:
    /** helper class for getting a character since that is a bit odd*/
    char getValueChar ();
    friend class ValueFederateManager;
};

/** class to handle an input and extract a specific type
@tparam X the class of the value associated with a subscription*/
template <class X>
class InputT : public Input
{
  public:
  private:
    std::function<void(X, Time)> value_callback;  //!< callback function for the federate
    std::function<double(const X &v1, const X &v2)>
      changeDetectionOperator;  //!< callback function for change detection
    // determine if we can convert to a primary type
    using is_convertible_to_primary_type =
      std::conditional_t<((helicsType<X> () != helics_type_t::helicsCustom) || (isConvertableType<X> ())),
                         std::true_type,
                         std::false_type>;

  public:
    InputT () = default;
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the input
    @param[in] units the units associated with a Federate
    */
    InputT (ValueFederate *valueFed, const std::string &name, const std::string &units = std::string ())
        : Input (valueFed, name, ValueConverter<X>::type (), units)
    {
    }
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the input
    @param[in] units the units associated with a Federate
    */
    template <class FedPtr>
    InputT (FedPtr &valueFed, const std::string &name, const std::string &units = std::string ())
        : Input (valueFed, name, ValueConverter<X>::type (), units)
    {
    }

  public:
    /** get the most recent value
    @return the value*/
    X getValue () { return Input::getValue<X> (); }
    /** store the value in the given variable
    @param[out] out the location to store the value
    */
    void getValue (X &out) { Input::getValue<X> (out); }

    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void setInputNotificationCallback (std::function<void(X, Time)> callback)
    {
        value_callback = callback;
        fed->setInputNotificationCallback (*this, [=](Input &, Time time) { handleCallback (time); });
    }
    /** set a default value
    @param val the value to set as the default
    */
    void setDefault (const X &val) { Input::setDefault (val); }

  private:
    void handleCallback (Time time)
    {
        X out;
        Input::getValue (out);
        value_callback (out, time);
    }
};

template <class X>
void Input::getValue_impl (std::integral_constant<int, 0> /*V*/, X &out)
{
    if (fed->isUpdated (*this) || (hasUpdate && !changeDetectionEnabled))
    {
        auto dv = fed->getValueRaw (*this);
        if (type == helics_type_t::helicsUnknown)
        {
            type = getTypeFromString (fed->getPublicationType (*this));
        }

        valueExtract (dv, type, out);
        if (changeDetectionEnabled)
        {
            if (changeDetected (lastValue, out, delta))
            {
                lastValue = make_valid (out);
            }
            else
            {
                valueExtract (lastValue, out);
            }
        }
        else
        {
            lastValue = make_valid (out);
        }
    }
    else
    {
        valueExtract (lastValue, out);
    }
    hasUpdate = false;
}

template <class X>
inline const X &getValueRefImpl (defV &val)
{
    valueConvert (val, helicsType<X> ());
    return mpark::get<X> (val);
}

template <>
inline const std::string &getValueRefImpl (defV &val)
{
    // don't convert a named point to a string
    if ((val.index () == namedPointLoc))
    {
        return mpark::get<named_point> (val).name;
    }
    else
    {
        valueConvert (val, helics_type_t::helicsString);
        return mpark::get<std::string> (val);
    }
}

template <class X>
const X &Input::getValueRef ()
{
    static_assert (std::is_same<typeCategory<X>, std::integral_constant<int, 0>>::value,
                   "calling getValue By ref must be with a primary type");
    if (fed->isUpdated (*this) || (hasUpdate && !changeDetectionEnabled))
    {
        auto dv = fed->getValueRaw (*this);
        if (type == helics_type_t::helicsUnknown)
        {
            type = getTypeFromString (fed->getPublicationType (*this));
        }

        if (changeDetectionEnabled)
        {
            X out;
            valueExtract (dv, type, out);
            if (changeDetected (lastValue, out, delta))
            {
                lastValue = make_valid (std::move(out));
            }
        }
        else
        {
            valueExtract (dv, type, lastValue);
        }
    }
    else
    {
		//TODO:: PT make some logic that it can get the raw data from the core again if it was converted already
    }

    return getValueRefImpl<remove_cv_ref<X>> (lastValue);
}

}  // namespace helics
