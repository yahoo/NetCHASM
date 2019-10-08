// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMAUXINFO_H_
#define HMAUXINFO_H_

#include <string>
#include <mutex>
#include <vector>
#include <map>
#include <shared_mutex>
#include <memory>

#include "HMIPAddress.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"

/*! Stores the supported aux info types. */
enum HM_AUX_TYPE : uint8_t
{
    HM_LOAD_FILE,
    HM_OOB_FILE
};

class HMAuxLoadFB;
class HMAuxOOB;

/*! Base class for all aux information. Derive new aux types from this base. */
class HMAuxBase
{
public:
    virtual ~HMAuxBase() {}

    //! Static function to create a new aux object from a raw stream. */
    /*!
         Function used to de-serialize a raw stream. The derived type is automatically determined.
         When adding an aux type, implement a call to the drived class's internalDeserialize function.
         The first byte should always be HM_AUX_TYPE allowing the function to determine the type.
         \param buf the buffer holding the raw stream.
         \param size the size of the raw buffer.
         \return A unique pointer to the Aux Info de-serialized from the raw buffer.
     */
    static std::unique_ptr<HMAuxBase> deserialize(char* buf, uint32_t size);

    //! Function to clone the current Aux Info.
    /*!
     The clone function creates the correct copy from the derived class.
     \return A unique pointer to the new copy of the aux info.
     */
    virtual std::unique_ptr<HMAuxBase> clone() const = 0;
    //! Function to move the current Aux Info.
    /*
     The transfer function moves the aux info without slicing the derived class.
     \return A unique pointer to the moved aux info.
     */
    virtual std::unique_ptr<HMAuxBase> transfer() const = 0;
    //! Function to serialize the current aux info into a raw buffer.
    /*!
     The serialize function supports two types of calls designed to be called consecutively.
     When called with a null buf and size 0 serialize will return the required size of the buf to store the aux info.
     When called with a non-null buf and the correct size, serialize will store the aux info into the buf.
     \param buf pass nullptr to get the required size or a raw buffer to fill.
     \param size pass 0 to get the required size or the required size to fill the buffer.
     \return The required size of the buf or the number of bytes saved to the buffer.
     */
    virtual uint32_t serialize(char* buf, uint32_t size) const = 0;
    //! Prints the aux info to a string for logging.
    virtual std::string print() = 0;
    //! Return a LFB subtype of nullptr if this is not a LFB type.
    virtual HMAuxLoadFB* getLFB() = 0;
    //! Return a OOB type or nullptr if this is not an OOB type.
    virtual HMAuxOOB* getOOB() = 0;

    HM_AUX_TYPE m_type;
    std::string m_host;
    std::string m_resource;
    HMIPAddress m_ip;
    HMTimeStamp m_ts;

protected:
    //! De-serialize the raw buffer.
    /*!
     This function is called internally after the static de-serialize determines the class type.
     It need to fill in the internal data from the raw buffer.
     \param buf raw buffer to deserialize.
     \param size the size of the raw buffer.
     \return true if the deserialize was a success.
     */
    virtual bool internalDeserialize(char* buf, uint32_t size) = 0;
};

//! Store load feedback information
/*
     This aux info is to store the load feedback. This includes the load, target load and max load.
 */
class HMAuxLoadFB : public HMAuxBase
{
public:
    //! Function to clone the current Aux Info.
    /*!
         The clone function creates the correct copy from the derived class.
         \return A unique pointer to the new copy of the aux info.
     */
    std::unique_ptr<HMAuxBase> clone() const;
    //! Function to move the current Aux Info.
    /*
         The transfer function moves the aux info without slicing the derived class.
         \return A unique pointer to the moved aux info.
     */
    std::unique_ptr<HMAuxBase> transfer() const;
    //! Function to serialize the current aux info into a raw buffer.
    /*!
         The serialize function supports two types of calls designed to be called consecutively.
         When called with a null buf and size 0 serialize will return the required size of the buf to store the aux info.
         When called with a non-noll buf and the correct size, serialize will store the aux info into the buf.
         \param buf pass nullptr to get the required size or a raw buffer to fill.
         \param size pass 0 to get the required size or the required size to fill the buffer.
         \return The required size of the buf or the number of bytes saved to the buffer.
     */
    uint32_t serialize(char* buf, uint32_t size) const;
    //! Prints the aux info to a string for logging.
    std::string print();
    //! Return a LFB subtype of nullptr if this is not a LFB type.
    HMAuxLoadFB* getLFB();
    //! Return a OOB type or nullptr if this is not an OOB type.
    HMAuxOOB* getOOB();

    std::string m_datacenter;
    int64_t m_load;
    int64_t m_target;
    int64_t m_max;

protected:
    //! De-serialize the raw buffer.
    /*!
         This function is called internally after the static de-serialize determines the class type.
         It need to fill in the internal data from the raw buffer.
         \param buf raw buffer to deserialize.
         \param size the size of the raw buffer.
         \return true if the deserialize what a success.
     */
    bool internalDeserialize(char* buf, uint32_t size);

    struct SerStruct
    {
        uint8_t m_type;
        uint32_t m_hostSize;
        uint32_t m_resourceSize;
        HMIPAddress m_ip;
        uint64_t m_ts;
        uint32_t m_datacenterSize;
        int64_t m_load;
        int64_t m_target;
        int64_t m_max;
    };
};

//! Store the out of band information
/*!
     This function stores the out of band information including if the host is forced down and shed percentage.
 */
class HMAuxOOB : public HMAuxBase
{
public:
    //! Function to clone the current Aux Info.
    /*!
         The clone function creates the correct copy from the derived class.
         \return A unique pointer to the new copy of the aux info.
     */
    std::unique_ptr<HMAuxBase> clone() const;
    //! Function to move the current Aux Info.
    /*
         The transfer function moves the aux info without slicing the derived class.
         \return A unique pointer to the moved aux info.
     */
    std::unique_ptr<HMAuxBase> transfer() const;
    //! Function to serialize the current aux info into a raw buffer.
    /*!
         The serialize function supports two types of calls designed to be called consecutively.
         When called with a null buf and size 0 serialize will return the required size of the buf to store the aux info.
         When called with a non-noll buf and the correct size, serialize will store the aux info into the buf.
         \param buf pass nullptr to get the required size or a raw buffer to fill.
         \param size pass 0 to get the required size or the required size to fill the buffer.
         \return The required size of the buf or the number of bytes saved to the buffer.
     */
    uint32_t serialize(char* buf, uint32_t size) const;
    //! Prints the aux info to a string for logging.
    std::string print();
    //! Return a LFB subtype of nullptr if this is not a LFB type.
    HMAuxLoadFB* getLFB();
    //! Return a OOB type or nullptr if this is not an OOB type.
    HMAuxOOB* getOOB();

    bool m_forceDown;
    uint32_t m_shed;

protected:
    //! De-serialize the raw buffer.
    /*!
         This function is called internally after the static de-serialize determines the class type.
         It need to fill in the internal data from the raw buffer.
         \param buf raw buffer to deserialize.
         \param size the size of the raw buffer.
         \return true if the deserialize what a success.
     */
    bool internalDeserialize(char* buf, uint32_t size);

    struct SerStruct
    {
        uint8_t m_type;
        uint32_t m_hostSize;
        uint32_t m_resourceSize;
        HMIPAddress m_ip;
        uint64_t m_ts;
        bool m_forceDown;
        uint32_t m_shed;

    };
};

//! Class to be used internally as the key to the map. Lookup by hostname, src URL and IPAddress.
class HMAuxKey
{
public:
    bool operator<(const HMAuxKey& k) const;
    uint32_t serialize(char* buf, uint32_t size) const;
    bool deserialize(char* buf, uint32_t size);

    std::string m_hostName;
    std::string m_sourceUrl;
    HMIPAddress m_address;
};

//! Class to store the aux info internally. Supports a vector of Aux ino and the update timestamp.
class HMAuxInfo
{
public:
    HMAuxInfo() {};

    HMAuxInfo(const HMAuxInfo& k);
    HMAuxInfo(const HMAuxInfo&& k);
    HMAuxInfo& operator=(const HMAuxInfo& k);
    HMAuxInfo& operator=(const HMAuxInfo&& k);

    std::vector<std::unique_ptr<HMAuxBase>> m_auxData;
    HMTimeStamp m_ts;
};

//! Main internal Aux Cache class.
/*!
     Main internal Aux Cache class. Supports looking up Aux info by hostname, ip, url tuple.
     Stores a vector of Aux base classes per tuple along with the update timestamp.
     All Aux info should be stored internally inside this class.
 */
class HMAuxCache
{
public:
    HMAuxCache() {}
    HMAuxCache(HMAuxCache&) = delete;

    //! Store the aux info read from an xml string
    /*
     Store parses the aux info from an input string. This currently supports xml parsing only.
     \param hostname the hostname of the source of the aux info string.
     \param sourceURL the URL used to retrieve the aux info string
     \param address the ip address of the host that provided the aux info string.
     \param actual aux info string to parse.
     \param aux info string format
     \return bool true if the parsing resulted in an aditional aux info entry added to the cache.
     */
    bool storeAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            std::string& auxInfo,
            HM_AUX_DATA_TYPE auxDataType);

    //! Store the aux info read from an xml string
    /*
         Store aux info
         \param hostname the hostname of the source of the aux info string.
         \param sourceURL the URL used to retrieve the aux info string
         \param address the ip address of the host that provided the aux info string.
         \param auxInfo the actual aux info.
         \return bool true if successful.
     */

    bool storeAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            HMAuxInfo& auxInfo);

    //! Update a current aux info entry
    /*
      Update the aux info to the provided auxInfo to the function.
      \param  hostname the hostname entry to update.
      \param sourceURL the sourceURL entry to update.
      \param address the ip address entry to update.
      \param auxInfo the auxInfo to use to replace the current auxInfo.
      \return bool true if the entry was updated.
     */
    bool updateAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            HMAuxInfo& auxInfo);

    //! Get the aux info for the provided hostname, sourceURL, address.
    /*
         Get the aux info data for the provided hostname, sourceURL and address.
         \param the hostname to lookup.
         \param the sourceURL to lookup.
         \param the address to lookup.
         \param the auxInfo data structure to fill used to return the aux info.
         \return bool true if the key existed and the auxInfo was successfully filled.
     */
    bool getAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            HMAuxInfo& auxInfo);

    //! Generate an xml representation of the stored aux info.
    /*
          Generate an xml representation of the aux info.
          \param the auxInfo to convert to xml.
          \param the HM_AUX_TYPE of the given aux info.
          \param the hostGroup to use in naming the xml.
          \param the string to store the xml.
          \param format to generate the aux info to
          \return bool true if the xml was stored into the string.
     */
    bool genAuxData(HMAuxInfo& auxInfo,
            const HM_AUX_TYPE type,
            const std::string& hostGroup,
            std::string& output,
            HM_AUX_DATA_TYPE auxDataType);

private:

    //! Internal function to parse the xml into a new entry in the cache
    /*!
         Parse the given string into a new cache entry.
         \param hostname the hostname to use in the key.
         \param sourceURL the url to use in the key.
         \param address the address to use in the key.
         \param string containing the xml to parse.
         \param auxInfo to store the xml.
         \param format to parse the aux info from
         \return bool true if the xml resulted in a new cache entry,
     */
    bool parseAuxData(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            std::string& auxStr,
            HMAuxInfo& auxInfo,
            HM_AUX_DATA_TYPE auxDataType);

    //! Internal function to lock the structure and replace a key-value pair.
    void commitEntry(HMAuxKey& key,
            HMAuxInfo& base);

    std::map<HMAuxKey, HMAuxInfo> m_auxData;
    std::shared_timed_mutex m_sharedMutex;

};

#endif /* HMAUXINFO_H_ */
