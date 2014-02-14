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
 * validate that the iterator initializes to provide elements.
 */

#include <config.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>
#include "boost_fix.hpp"
#include "to_key_helper.hpp"
#include "hashdb_element.hpp"
#include "dfxml_hashdigest_reader_manager.hpp"

/*
static const char temp_dir[] = "temp_dir";
static const char temp_map[] = "temp_dir/hash_store";
static const char temp_multimap[] = "temp_dir/hash_duplicates_store";
*/

void run_tests() {

  dfxml_hashdigest_reader_manager_t manager("sample_dfxml", "my repository");

  std::vector<hashdb_element_t>::const_iterator it = manager.begin();

  size_t count = 0;
  while (it != manager.end()) {
std::cout << "value: " << it->hashdigest << "\n";
    ++count;
    ++it;
  }
  BOOST_TEST_EQ(count, 75);
}

int cpp_main(int argc, char* argv[]) {

  run_tests();

  // done
  int status = boost::report_errors();
  return status;
}
