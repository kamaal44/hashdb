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
 * Provides repository name, filename to source lookup index lookup
 * using LMDB.
 *
 * This module is not threadsafe.
 * Locks are required around contexts that can write to preserve
 * integrity, in particular to allow grow and to preserve accurate size.
 */

#ifndef LMDB_NAME_STORE_HPP
#define LMDB_NAME_STORE_HPP
#include <string>
#include "lmdb.h"
#include "lmdb_helper.h"
#include "lmdb_data_codec.hpp"
#include "lmdb_context.hpp"

class lmdb_name_store_t {

  private:
  const std::string hashdb_dir;
  const file_mode_type_t file_mode;
  MDB_env* env;

  // disallow these
  lmdb_name_store_t(const lmdb_name_store_t&);
  lmdb_name_store_t& operator=(const lmdb_name_store_t&);

  public:
  lmdb_name_store_t (const std::string p_hashdb_dir,
                     file_mode_type_t p_file_mode) :

       hashdb_dir(p_hashdb_dir),
       file_mode(p_file_mode),
       env(0) {

    // create the DB environment
    MDB_env* new_env;
    int rc = mdb_env_create(&new_env);
    if (rc != 0) {
      // bad failure
      assert(0);
    }

    // the DB stage directory
    const std::string store_dir = hashdb_dir + "/lmdb_name_store";

    // open the DB environment
    env = lmdb_helper::open_env(store_dir, file_mode);
  }

  ~lmdb_name_store_t() {

    // close the MDB environment
    mdb_env_close(env);
  }

  /**
   * Insert and return true and the next source lookup index
   * else return false and existing source_lookup_index if already there.
   */
  std::pair<bool, uint64_t> insert(const std::string& repository_name,
                                   const std::string& filename) {

    // maybe grow the DB
    lmdb_helper::maybe_grow(env);

    // get context
    lmdb_context_t context(env, true, false);
    context.open();

    // encode the key
    std::string key_encoding = lmdb_data_codec::encode_name_data(
                                            repository_name, filename);

    lmdb_helper::point_to_string(key_encoding, context.key);

    // see if key is already there
    int rc = mdb_cursor_get(context.cursor, &context.key, &context.data,
                            MDB_SET_KEY);
    bool is_new = false;
    uint64_t source_lookup_index;
    if (rc == 0) {
      // great, get the existing index
      std::string encoding = lmdb_helper::get_string(context.data);
      source_lookup_index = lmdb_data_codec::decode_uint64_data(encoding);
    } else if (rc == MDB_NOTFOUND) {
      // fine, add new entry
      is_new = true;
      source_lookup_index = size() + 1;
      std::string data_encoding = lmdb_data_codec::encode_uint64_data(source_lookup_index);
      lmdb_helper::point_to_string(data_encoding, context.data);
      rc = mdb_put(context.txn, context.dbi, &context.key, &context.data,
                                                         MDB_NOOVERWRITE);
      if (rc != 0) {
        std::cerr << "name insert failure: " << mdb_strerror(rc) << "\n";
        assert(0);
      }

    } else {
      std::cerr << "name lookup failure: " << mdb_strerror(rc) << "\n";
      source_lookup_index = 0; // to suppress mingw warning
      assert(0);
    }

    context.close();

    return std::pair<bool, uint64_t>(is_new, source_lookup_index);
  }

  /**
   * Find index, return true and index else false and zero.
   */
  std::pair<bool, uint64_t> find(const std::string& repository_name,
                                 const std::string& filename) const {

    // get context
    lmdb_context_t context(env, false, false);
    context.open();

    // encode the key
    std::string encoding = lmdb_data_codec::encode_name_data(
                                            repository_name, filename);
    lmdb_helper::point_to_string(encoding, context.key);

    // see if key is there
    int rc = mdb_cursor_get(context.cursor, &context.key, &context.data,
                            MDB_SET_KEY);

    bool is_there = false;
    uint64_t source_lookup_index = 0;
    if (rc == 0) {
      // great, get the existing index
      std::string data_encoding = lmdb_helper::get_string(context.data);
      source_lookup_index = lmdb_data_codec::decode_uint64_data(data_encoding);
      is_there = true;
    } else if (rc == MDB_NOTFOUND) {
      // no action
    } else {
      std::cerr << "name find failure: " << mdb_strerror(rc) << "\n";
      assert(0);
    }

    context.close();
    return std::pair<bool, uint64_t>(is_there, source_lookup_index);
  }

  // size
  size_t size() const {
    return lmdb_helper::size(env);
  }
};

#endif

