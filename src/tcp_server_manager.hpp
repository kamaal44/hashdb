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
 * Provides the server hashdb scan service using Boost.Asio.
 * Modeled after the Boost blocking_tcp_echo_server.cpp example.
 */

#ifndef TCP_SERVER_MANAGER_HPP
#define TCP_SERVER_MANAGER_HPP

#include <config.h>
#include "hashdb_manager.hpp"
#include "file_modes.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <dfxml/src/hash_t.h>

// types of queries that are available
const uint32_t QUERY_MD5 = 1;
const uint32_t QUERY_SHA1 = 2;
const uint32_t QUERY_SHA256 = 3;

class tcp_server_manager_t {

  private:
  hashdb_manager_t hashdb_manager;
  uint32_t hashdb_hashdigest_type;
  boost::asio::io_service io_service;
  boost::mutex scan_mutex;

  typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr_t;

  // get the hashdigest type used by hashdb_manager
  uint32_t get_hashdb_hashdigest_type() {
    if (hashdb_manager.hashdigest_type_string() == "MD5") {
      return QUERY_MD5;
    } else if (hashdb_manager.hashdigest_type_string() == "SHA1") {
      return QUERY_SHA1;
    } else if (hashdb_manager.hashdigest_type_string() == "SHA256") {
      return QUERY_SHA256;
    } else {
      assert(0); exit(1); // program error
    }
  }

  // run a complete connection session.
  // The session is run on a thread by a server dispatcher.
  void run_session(socket_ptr_t socket_ptr) {
    try {
      // loop servicing request/response cycles based on the hashdigest type
      // the hashdb_manager supports
      bool valid_session = true;
      while(valid_session) {
        // perform scan iteration based on request type
        if (hashdb_hashdigest_type == QUERY_MD5) {
          valid_session = do_scan<md5_t>(socket_ptr);
        } else if (hashdb_hashdigest_type == QUERY_SHA1) {
          valid_session = do_scan<sha1_t>(socket_ptr);
        } else if (hashdb_hashdigest_type == QUERY_SHA256) {
          valid_session = do_scan<sha256_t>(socket_ptr);
        } else {
          assert(0); exit(1); // program error
        }
      }
    } catch (std::exception& e) {
      std::cerr << "Exception in request, request dropped: " << e.what() << "\n";
    }
  }

  // perform one request/response scan iteration
  // true = more, false=EOF
  template<typename T>
  bool do_scan(socket_ptr_t socket_ptr) {
    typedef std::vector<std::pair<uint64_t, T> > request_t;
    
    // read the client's hashdigest type, acknowledging EOF
    uint32_t client_hashdigest_type;
    boost::system::error_code error;
    boost::asio::read(*socket_ptr, boost::asio::buffer(
                    &client_hashdigest_type, sizeof(client_hashdigest_type)),
                    error);

    // the protocol may leave the scan loop here
    if (error == boost::asio::error::eof) {
      // done
      return false;
    } else if (error) {
      throw boost::system::system_error(error); // Some other error.
    }


    // check for hashdigest compatibility else drop this connection.
    if (client_hashdigest_type != hashdb_hashdigest_type) {
      std::cout << "tcp_server_manager: client and server hashdigest types do not match.  Request dropped.\n";
      return false;
    }

    // read the request size
    uint32_t request_size;
    boost::asio::read(*socket_ptr, boost::asio::buffer(
                                    &request_size, sizeof(request_size)));

    // allocate request and response vectors on heap
    boost::shared_ptr<request_t> request_ptr(new request_t(request_size));
    boost::shared_ptr<hashdb_t::scan_output_t>
                         response_ptr(new hashdb_t::scan_output_t);

    // read the request
    boost::asio::read(*socket_ptr, boost::asio::buffer(*request_ptr));

    // lock this until we are confident that reading is threadsafe
    // use boost mutex since this will be running on a boost-launched thread
    scan_mutex.lock();

    // scan each input in turn
    typename request_t::const_iterator it = request_ptr->begin();
    while (it != request_ptr->end()) {
      uint32_t count = hashdb_manager.find_count(it->second);
      if (count > 0) {
        std::pair<uint64_t, uint32_t> return_item(it->first, count);
        response_ptr->push_back(return_item);
      }
      ++it;
    }

    // unlock
    scan_mutex.unlock();

    // send the response count
    uint32_t response_size = response_ptr->size();
    boost::asio::write(*socket_ptr, boost::asio::buffer(
                                    &response_size, sizeof(response_size)));

    // send the response vector
    boost::asio::write(*socket_ptr, boost::asio::buffer(*response_ptr));

    return true;
  }

  public:
  /**
   * This works as follows:
   *   initialize state,
   *   while true:
   *     wait to accept a socket connection,
   *     dispatch the connection to service request, scan, response queries.
   *
   * Note that the service thread intentionally runs on the
   * tcp_server_manager_t singleton in order to have visibility to
   * its resources, and that the singleton must access threadsafe members.
   */
  tcp_server_manager_t(const std::string& hashdb_dir, uint16_t port_number) :
                       hashdb_manager(hashdb_dir, READ_ONLY),
                       hashdb_hashdigest_type(get_hashdb_hashdigest_type()),
                       io_service(),
                       scan_mutex() {

    try {
      boost::asio::ip::tcp::acceptor acceptor(io_service,
                  boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                 port_number));
      while (true) {
        socket_ptr_t socket_ptr(new boost::asio::ip::tcp::socket(io_service));
        acceptor.accept(*socket_ptr);

        // now run this.run_session on a thread.
        // this should be safe since the resources it accesses are threadsafe.
        boost::thread t(boost::bind(&tcp_server_manager_t::run_session,
                                    this,
                                    socket_ptr));
      }
    } catch (std::exception& e) {
      std::cerr << "Exception: unable to connect to socket.  Aborting.  " << e.what() << "\n";
    }
  }
};

#endif

