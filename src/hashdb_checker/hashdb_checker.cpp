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
 * Provides the main entry for the hashdb_checker tool that exists
 * to provide dfxml lookup services and to test hashdb interfaces.
 */

#include <config.h>
#ifdef WIN32
  // including winsock2.h now keeps an included header somewhere from
  // including windows.h first, resulting in a warning.
  #include <winsock2.h>
#endif
// Standard includes
#include <cstdlib>
#include <cstdio>
#include <string>
#include "hashdb.hpp"
#include "dfxml_hashdigest_reader.hpp"
#include "hash_lookup_consumer.hpp"
//#include "md5.h"
#include <getopt.h>

//void do_show_hashdb_info(lookup_type_t lookup_type, std::string lookup_path);
void do_hash_lookup_md5(hashdb::lookup_type_t lookup_type,
                        std::string lookup_path,
                        std::string dfxml_infile);

static std::string see_usage = "Please type 'hashdb_checker -h' for usage.";

// options
bool has_lookup_type = false;
bool has_client_hashdb_path = false;
bool has_client_socket_endpoint = false;
std::string client_hashdb_path = "hashdb path not defined";
std::string client_socket_endpoint = "tcp://localhost:14500";

void usage() {
  std::cout
  << "hashdb_checker version " << PACKAGE_VERSION  << "\n"
  << "Usage: hashdb_checker -h | -v | <command>\n"
  << "    -h, --help    print this message\n"
  << "    --Version     print version number\n"
  << "\n"
  << "hashdb_checker supports the following <command> options:\n"
  << "\n"
  << "--info [<lookup parameter>]+\n"
  << "\n"
  << "    Options:\n"
  << "    <lookup parameter>\n"
  << "        Please see <lookup parameter> options.\n"
  << "\n"
  << "--lookup [<lookup parameter>]+ <dfxml input>\n"
  << "\n"
  << "    Options:\n"
  << "    <lookup parameter>\n"
  << "        Please see <lookup parameter> options.\n"
  << "\n"
  << "    Parameters:\n"
  << "        <dfxml input>  a DFXML file containing hashes to be looked up\n"
  << "\n"
  << "<lookup parameter> options establish the lookup type and location:\n"
  << "    -l, --lookup_type=<lookup type>\n"
  << "        <lookup type> used to perform the lookup, where <lookup_type>\n"
  << "        is one of use_path | use_socket (default use_path).\n"
  << "        use_path   - Lookups are performed from a hashdb in the filesystem\n"
  << "                     at the specified <path>.\n"
  << "        use_socket - Lookups are performed from a server service at the\n"
  << "                     specified <socket>.\n"
  << "\n"
  << "    -p, --path =<path>]\n"
  << "        specifies the <path> to the hash database to be used for performing\n"
  << "        the lookup service. This option is only valid when the lookup type\n"
  << "        is set to \"use_path\".\n"
  << "\n"
  << "    -s, --socket=<socket>]\n"
  << "        specifies the client <socket> endpoint to use to connect with the\n"
  << "        hashdb server (default '" << client_socket_endpoint << "').  Valid socket\n"
  << "        transports supported by the zmq messaging kernel are tcp, ipc, and\n"
  << "        inproc.  Currently, only tcp is tested.  This option is only valid\n"
  << "        when the lookup type is set to \"use_socket\".\n"
  << "\n"
  ;
}

int main(int argc,char **argv)
{

  // input parsing commands
  int info_flag = 0;
  int lookup_flag = 0;

  // defaults
  hashdb::lookup_type_t lookup_type = hashdb::QUERY_USE_PATH;
  std::string lookup_path = "query path not defined";

  // manage when there are no arguments
  if(argc==1) {
      usage();
      exit(1);
  }

  // parse options
  int option_index; // not used
  while (1) {

    static struct option long_options[] = {
      // general
      {"help", no_argument, 0, 'h'},
      {"Version", no_argument, 0, 'V'},

      // commands
      {"info", no_argument, &info_flag, 1},
      {"lookup", no_argument, &lookup_flag, 1},

      // command options
      {"lookup_type", required_argument, 0, 'l'},
      {"path", required_argument, 0, 'p'},
      {"socket", required_argument, 0, 's'},

      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hVl:p:s:", long_options, &option_index);
    if (ch == -1) {
      // no more arguments
      break;
    }
    if (ch == 0) {
      // command options set flags and use ch==0
      continue;
    }
    switch (ch) {
      case 'h': {	// help
        usage();
        exit(0);
        break;
      }
      case 'V': {	// Version
        std::cout << "hashdb_checker " << PACKAGE_VERSION << "\n";
        exit(0);
        break;
      }
      case 'l': {	// lookup type
        has_lookup_type = true;
        bool is_ok_lookup_type = string_to_lookup_type(optarg, lookup_type);
        if (!is_ok_lookup_type) {
          std::cerr << "Invalid lookup type: '" << optarg << "'.  " << see_usage << "\n";
          exit(1);
        }
        break;
      }
      case 'p': {	// client hashdb path
        has_client_hashdb_path = true;
        client_hashdb_path = optarg;
        break;
      }
      case 's': {	// client socket endpoint
        has_client_socket_endpoint = true;
        client_socket_endpoint = optarg;
        break;
      }
      default:
//        std::cerr << "unexpected command character " << ch << "\n";
        exit(1);
    }
  }

  // check that there is exactly one command issued
  int num_commands = info_flag + lookup_flag;
  if (num_commands == 0) {
    std::cerr << "Error: missing command.  " << see_usage << "\n";
    exit(1);
  }
  if (num_commands > 1) {
    std::cerr << "Only one command may be requested.  " << see_usage << "\n";
    exit(1);
  }

  // set lookup path based on lookup type
  switch(lookup_type) {
    case hashdb::QUERY_USE_PATH:   lookup_path = client_hashdb_path;     break;
    case hashdb::QUERY_USE_SOCKET: lookup_path = client_socket_endpoint; break;
    default:            lookup_path = "lookup path not defined";
  }

  // if the lookup type is QUERY_USE_PATH then the lookup path must be provided
  if (lookup_type == hashdb::QUERY_USE_PATH && has_client_hashdb_path == false) {
    std::cerr << "The --path parameter is required when the lookup type is 'lookup_path'\n";
    exit(1);
  }

 
  // allocate any mandatory arguments
  std::string arg1 = "";

  // run the command

  // info
  if (info_flag) {
    if (argc - optind != 0) {
      std::cerr << "The info command requires 0 parameters.  " << see_usage << "\n";
      exit(1);
    }
 
//    do_show_hashdb_info(lookup_type, lookup_path);
    std::cout << "info currently not supported.\n";

  // lookup
  } else if (lookup_flag) {
    if (argc - optind != 1) {
      std::cerr << "The lookup command requires 1 parameter.  " << see_usage << "\n";
      exit(1);
    }
    arg1 = argv[optind++];
 
    if (has_client_hashdb_path && has_client_socket_endpoint) {
      std::cerr << "A path or a socket may be selected, but not both.  " << see_usage << "\n";
      exit(1);
    }
    do_hash_lookup_md5(lookup_type, lookup_path, arg1);

  } else {
    // program error
    assert(0);
  }

  // done
  return 0;
}

/*
void do_show_hashdb_info(hashdb::lookup_type_t lookup_type, std::string lookup_path) {
  std::cout << "hashdb info, lookup type '" << lookup_type_to_string(lookup_type)
            << "', lookup path '" << lookup_path << "'\n";

  // allocate space for the response
  query_info_response_t response;

  // create the client query service
  hashdb::query_t query(lookup_type, lookup_path);

  // perform the information lookup
  bool success = query.get_hashdb_info(response);
  if (success) {
    std::cout << "report:\n";
    response.hashdb_settings.report_settings(std::cout);
    std::cout << "\n";
  } else {
    std::cerr << "Failure in accessing the hashdb server for info.\n";
  }
}
*/

void do_hash_lookup_md5(hashdb::lookup_type_t lookup_type,
                        std::string lookup_path,
                        std::string dfxml_infile) {
  std::cout << "hashdb lookup, lookup type " << lookup_type_to_string(lookup_type)
            << " lookup path '" << lookup_path << "'\n";

  // request, response, and source text map
  hashdb::hashes_request_md5_t request;
  hashdb::hashes_response_md5_t response;
  std::map<uint32_t, std::string> source_map;

  // create the hash lookup consumer with its consume callback method
  hash_lookup_consumer_t hash_lookup_consumer(&request, &source_map);

  // perform the DFXML read
  dfxml_hashdigest_reader_t<hash_lookup_consumer_t>::do_read(
                             dfxml_infile, "not used", &hash_lookup_consumer);

  // create the client query service
  hashdb::query_t query(lookup_type, lookup_path);

  // perform the query lookup
  bool success = query.lookup_hashes_md5(request, response);

  // show result
  if (success) {
    for (std::vector<hashdb::hash_response_md5_t>::const_iterator it = response.hash_responses.begin(); it != response.hash_responses.end(); ++it) {

      const uint8_t* digest = it->digest;
      md5_t md5;
      memcpy(md5.digest, digest, 16);

      std::cout << source_map[it->id] << "\t"
                << md5 << "\t"
                << "count=" << it->duplicates_count
                << ",source_lookup_index=" << it->source_lookup_index
                << ",chunk_offset_value=" << it->chunk_offset_value
                << ",from_map=" << source_map[it->id]
                << "\n";
    }
  } else {
    std::cerr << "Failure in accessing the hashdb server for lookup.\n";
  }
}

