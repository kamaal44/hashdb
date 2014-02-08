// Author:  Bruce Allen <bdallen@nps.edu>
// Created: 2/25/2013
//
// The software provided here is released by the Naval Postgraduate
// School, an agency of the U.S. Department of Navy.  The software
// bears no warranty, either expressed or implied. NPS does not assume
// legal liability nor responsibility for a User's use of the software
// or the results of such use.
//
// Please note that within the United States, copyright protection,
// under Section 105 of the United States Code, Title 17, is not
// available for any work of the United States Government and/or for
// any works created by United States Government employees. User
// acknowledges that this software contains work which was created by
// NPS government employees and is therefore in the public domain and
// not subject to copyright.
//
// Released into the public domain on February 25, 2013 by Bruce Allen.

/**
 * \file
 * Provides a hashdb iterator which wraps map_multimap_iterator_t<T>.
 * Dereferences to pair<hexdigest_string, source_lookup_encoding>
 */

#ifndef HASHDB_ITERATOR_HPP
#define HASHDB_ITERATOR_HPP
#include "map_multimap_iterator.hpp"
#include "dfxml/src/hash_t.h"
#include "hashdigest_types.h"

class hashdb_iterator_t {
  private:

  // the hashdigest type that this iterator will be using
  hashdigest_type_t hashdigest_type;

  // the hashdigest-specific iterators, one of which will be used,
  // based on how hashdb_iterator_t is instantiated
  map_multimap_iterator_t<md5_t> md5_iterator;
  map_multimap_iterator_t<sha1_t> sha1_iterator;
  map_multimap_iterator_t<sha256_t> sha256_iterator;

  // the dereferenced value of hexdigest string, source_lookup_encoding
  std::pair<std::string, uint64_t> dereferenced_value;

  // elemental forward iterator accessors are increment, equal, and dereference
  // increment
  void increment() {
    switch(hashdigest_type) {
      case HASHDIGEST_MD5:     ++md5_iterator; return;
      case HASHDIGEST_SHA1:    ++sha1_iterator; return;
      case HASHDIGEST_SHA256:  ++sha256_iterator; return;
      default: assert(0);
    }
  }

  // equal
  bool equal(hashdb_iterator_t const& other) const {
    // it is a program error if hashdigest types differ
    if (this->hashdigest_type != other.hashdigest_type) {
      assert(0);
    }

    switch(hashdigest_type) {
      case HASHDIGEST_MD5:
        return this->md5_iterator == other.md5_iterator;
      case HASHDIGEST_SHA1:
        return this->sha1_iterator == other.sha1_iterator;
      case HASHDIGEST_SHA256:
        return this->sha256_iterator == other.sha256_iterator;
      default: assert(0);
    }
  }

  // dereference
  void dereference() {
    std::string hexdigest_string;
    switch(hashdigest_type) {
      case HASHDIGEST_MD5:
        dereferenced_value = std::pair<std::string, uint64_t>(
                    md5_iterator->first.hexdigest(),
                    md5_iterator->second); break;
      case HASHDIGEST_SHA1:
        dereferenced_value = std::pair<std::string, uint64_t>(
                    sha1_iterator->first.hexdigest(),
                    sha1_iterator->second); break;
      case HASHDIGEST_SHA256:
        dereferenced_value = std::pair<std::string, uint64_t>(
                    sha256_iterator->first.hexdigest(),
                    sha256_iterator->second); break;
      default: assert(0);
    }
  }

  public:
  // the constructors for each map_multimap type using native iterators
  hashdb_iterator_t(map_multimap_iterator_t<md5_t> p_it) :
                      hashdigest_type(HASHDIGEST_MD5),
                      md5_iterator(p_it),
                      sha1_iterator(),
                      sha256_iterator(),
                      dereferenced_value() {
  }

  // this useless default constructor is required by std::pair
  hashdb_iterator_t() :
                      hashdigest_type(HASHDIGEST_UNDEFINED),
                      md5_iterator(),
                      sha1_iterator(),
                      sha256_iterator(),
                      dereferenced_value() {
  }

  // copy capability is required by std::pair
  hashdb_iterator_t& operator=(const hashdb_iterator_t& other) {
    hashdigest_type = other.hashdigest_type;
    md5_iterator = other.md5_iterator;
    sha1_iterator = other.sha1_iterator;
    sha256_iterator = other.sha256_iterator;
    dereferenced_value = other.dereferenced_value;
    return *this;
  }

  // the Forward Iterator concept consits of ++, *, ->, ==, !=
  hashdb_iterator_t& operator++() {
    increment();
    return *this;
  }
  hashdb_iterator_t operator++(int) {  // c++11 delete would be better.
    hashdb_iterator_t temp(*this);
    increment();
    return temp;
  }
  std::pair<std::string, uint64_t>& operator*() {
    dereference();
    return dereferenced_value;
  }
  std::pair<std::string, uint64_t>* operator->() {
    dereference();
    return &dereferenced_value;
  }
  bool operator==(const hashdb_iterator_t& other) const {
    return equal(other);
  }
  bool operator!=(const hashdb_iterator_t& other) const {
    return !equal(other);
  }
};

#endif
