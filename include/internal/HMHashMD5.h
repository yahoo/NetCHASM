// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMHASHMD5_H_
#define HMHASHMD5_H_

#include <string>
#include <openssl/evp.h>

//! Default size for MD5 hash
#define HASH_MAX_SIZE 64 //Max MD5 length EVP_MAX_MD_SIZE

//! Class to hold the hash value
/*!
     Class to hold the hash value
     HMHash stores the hash of configs loaded.
 */
class HMHash
{
public:
    HMHash(): m_hashSize(0) { }
    unsigned char m_hashValue[HASH_MAX_SIZE];
    uint32_t m_hashSize;
    bool operator==(const HMHash& k) const;
    bool operator!=(const HMHash& k) const;
};



//! The class computes MD5 hash NetCHASM.
/*!
    Computed hash is stored in md_value.
 */
class HMHashMD5
{
public:
   HMHashMD5();
   bool init();
   
   //! Update the value of the hash.
   /*!
        Update the value of the hash. Update can be called any number of to time add addition values to hash.
        \param the data to be hashed.
        \param the size of the data to hash in bytes.
    */
   void update(const void* data, size_t len);

   //! Final is called at the end , which computes the final hash.
    /*!
     Computes final hash.
     \param variable to store the hash value.
     */
   void final(HMHash &hash);
   
   //! Get the value of the hash.
   /*!
        Get the value of the hash.
        \return the hash value if computed successfully, else a nullptr.
    */
   char* get(void) const;

   //! Get the size of the hash.
   /*!
        Get the size of the hash.
        \return the size of the hash.
    */
   size_t size(void) const;

   //! Clear the value of the hash.
   ~HMHashMD5();

   EVP_MD_CTX* ctx;
   bool finalized;
};

#endif /* HMHASHMD5_H_ */
