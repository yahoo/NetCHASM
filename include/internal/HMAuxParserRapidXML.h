// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_INTERNAL_HMAUXPARSERRAPIDXML_H_
#define INCLUDE_INTERNAL_HMAUXPARSERRAPIDXML_H_


#include "HMAuxParser.h"
#include "rapidxml.h"

class HMAuxParserRapidXML : public HMAuxParser
{
public:

    //! Generate an xml representation of the stored aux info.
    /*
          Generate an xml representation of the aux info.
          /param the auxInfo to convert to xml.
          /param the HM_AUX_TYPE of the given aux info.
          /param the hostGroup to use in naming the xml.
          /param the string to store the xml.
          /return bool true if the xml was stored into the string.
     */
    bool genAuxData(HMAuxInfo& auxInfo,
            const HM_AUX_TYPE type,
            const std::string& hostGroup,
            std::string& xmlOutput);

    //! Internal function to parse the aux data into a new entry in the cache
    /*!
     Parse the given string into a new cache entry.
     \param hostname the hostname to use in the key.
     \param sourceURL the url to use in the key.
     \param address the address to use in the key.
     \param string containing the xml to parse.
     \param auxInfo to store the xml.
     \return bool true if the xml resulted in a new cache entry,
     */
    bool parseAuxData(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            std::string& auxString,
            HMAuxInfo& auxInfo);

    ~HMAuxParserRapidXML() {}

private:

    //! Internal function to parse the xml into a new LFB entry in the cache
    /*!
        Parse the given string into a new LFB cache entry.
        \param hostname the hostname to use in the key.
        \param sourceURL the url to use in the key.
        \param address the address to use in the key.
        \param string containing the xml to parse.
        \param auxInfo to store the xml.
        \return bool true if the xml resulted in a new LFB cache entry,
     */
    bool parseNewLFB(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            rapidxml::xml_document<char>& doc,
            HMAuxInfo& auxInfo);

    //! Internal function to parse the xml into an OOB entry in the cache
        /*!
            Parse the given string into an OOB cache entry.
            \param hostname the hostname to use in the key.
            \param sourceURL the url to use in the key.
            \param address the address to use in the key.
            \param string containing the xml to parse.
            \param auxInfo to store the xml.
            \return bool true if the xml resulted in an OOB cache entry,
         */
    bool parseOOB(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            rapidxml::xml_document<char>& doc,
            HMAuxInfo& auxInfo);


};






#endif /* INCLUDE_INTERNAL_HMAUXPARSERRAPIDXML_H_ */
