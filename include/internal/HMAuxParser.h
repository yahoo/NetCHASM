// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_INTERNAL_HMAUXPARSER_H_
#define INCLUDE_INTERNAL_HMAUXPARSER_H_

#include <iostream>

#include "HMAuxCache.h"
class HMAuxParser
{
public:

    //! Generate an writable representation of the stored aux info.
    /*
          Generate an xml representation of the aux info.
          /param the auxInfo to convert to xml.
          /param the HM_AUX_TYPE of the given aux info.
          /param the hostGroup to use in naming the xml.
          /param the string to store the xml.
          /return bool true if the xml was stored into the string.
     */
    virtual bool genAuxData(HMAuxInfo& auxInfo,
            const HM_AUX_TYPE type,
            const std::string& hostGroup,
            std::string& xmlOutput) = 0;

    //! Internal function to parse the aux data into a new entry in the cache
    /*!
     Parse the given string into a new cache entry.
     \param hostname the hostname to use in the key.
     \param sourceURL the url to use in the key.
     \param address the address to use in the key.
     \param string containing the xml to parse.
     \param auxInfo the string containing the xml to parse.
     \param auxInfo data structure to fill used to return the aux info.
     \return bool true if the xml resulted in a new cache entry,
     */
    virtual bool parseAuxData(const std::string& hostname,
            const std::string& sourceURL,
            const HMIPAddress& address,
            std::string& auxStr,
            HMAuxInfo& auxInfo) = 0;

    virtual ~HMAuxParser() {}

};



#endif /* INCLUDE_INTERNAL_HMAUXPARSER_H_ */
