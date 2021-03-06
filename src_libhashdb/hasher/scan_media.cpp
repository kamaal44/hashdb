// Author:  Bruce Allen
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
 * Support hashdb scan from media image.
 */
#include <config.h>
// this process of getting WIN32 defined was inspired
// from i686-w64-mingw32/sys-root/mingw/include/windows.h.
// All this to include winsock2.h before windows.h to avoid a warning.
#if defined(__MINGW64__) && defined(__cplusplus)
#  ifndef WIN32
#    define WIN32
#  endif
#endif
#ifdef WIN32
  // including winsock2.h now keeps an included header somewhere from
  // including windows.h first, resulting in a warning.
  #include <winsock2.h>
#endif

#include <string>
#include <cassert>
#include <iostream>
#include <unistd.h> // for F_OK
#include <sstream>
#include "num_cpus.hpp"
#include "hashdb.hpp"
#include "file_reader.hpp"
#include "hash_calculator.hpp"
#include "threadpool.hpp"
#include "job.hpp"
#include "job_queue.hpp"
#include "scan_tracker.hpp"
#include "tprint.hpp"

static const size_t BUFFER_DATA_SIZE = 16777216;   // 2^24=16MiB
static const size_t BUFFER_SIZE = 17825792;        // 2^24+2^20=17MiB
static const size_t MAX_RECURSION_DEPTH = 7;

namespace hashdb {
  // ************************************************************
  // scan_file
  // ************************************************************
  std::string scan_file(
        const hasher::file_reader_t& file_reader,
        hashdb::scan_manager_t& scan_manager,
        hasher::scan_tracker_t& scan_tracker,
        const size_t step_size,
        const size_t block_size,
        const bool process_embedded_data,
        const hashdb::scan_mode_t scan_mode,
        hasher::job_queue_t* const job_queue) {

    // identify the maximum recursion depth
    size_t max_recursion_depth = 
                        (process_embedded_data) ? MAX_RECURSION_DEPTH : 0;

    // create buffer b to read into
    size_t b_size = (file_reader.filesize <= BUFFER_SIZE) ?
                          file_reader.filesize : BUFFER_SIZE;
    uint8_t* b = new (std::nothrow) uint8_t[b_size]();
    if (b == NULL) {
      return "bad memory allocation";
    }

    // read into buffer b
    size_t bytes_read;
    std::string error_message;
    error_message = file_reader.read(0, b, b_size, &bytes_read);
    if (error_message.size() > 0) {
      // abort
      delete[] b;
      return error_message;
    }

    // build buffers from file sections and push them onto the job queue

    // push buffer b onto the job queue
    size_t b_data_size = (b_size > BUFFER_DATA_SIZE)
                         ? BUFFER_DATA_SIZE : b_size;
    job_queue->push(hasher::job_t::new_scan_job(
                 &scan_manager,
                 &scan_tracker,
                 step_size,
                 block_size,
                 file_reader.filename,
                 file_reader.filesize,
                 0,      // file_offset
                 process_embedded_data,
                 scan_mode,
                 b,      // buffer
                 b_size, // buffer_size
                 b_data_size, // buffer_data_size,
                 max_recursion_depth,
                 0,      // recursion_depth
                 ""));   // recursion path

    // read and push remaining buffers onto the job queue
    for (uint64_t offset = BUFFER_DATA_SIZE;
         offset < file_reader.filesize;
         offset += BUFFER_DATA_SIZE) {

      // create b2 to read into
      uint8_t* b2 = new (std::nothrow) uint8_t[BUFFER_SIZE]();
      if (b2 == NULL) {
        // abort
        return "bad memory allocation";
      }

      // read into b2
      size_t b2_bytes_read = 0;
      error_message = file_reader.read(
                                  offset, b2, BUFFER_SIZE, &b2_bytes_read);
      if (error_message.size() > 0) {
        // abort submitting jobs for this file
        delete[] b2;
        return error_message;
      }

      // push this buffer b2 onto the job queue
      size_t b2_data_size = (b2_bytes_read > BUFFER_DATA_SIZE)
                                        ? BUFFER_DATA_SIZE : b2_bytes_read;
      job_queue->push(hasher::job_t::new_scan_job(
                 &scan_manager,
                 &scan_tracker,
                 step_size,
                 block_size,
                 file_reader.filename,
                 file_reader.filesize,
                 offset,  // file_offset
                 process_embedded_data,
                 scan_mode,
                 b2,      // buffer
                 b2_bytes_read, // buffer_size
                 b2_data_size,  // buffer_data_size
                 max_recursion_depth,
                 0,      // recursion_depth
                 ""));   // recursion path
    }
    return "";
  }

  // ************************************************************
  // scan_media
  // ************************************************************
  std::string scan_media(const std::string& hashdb_dir,
                         const std::string& media_filename,
                         const size_t step_size,
                         const bool process_embedded_data,
                         const hashdb::scan_mode_t scan_mode) {

    // make sure hashdb_dir is there
    std::string error_message;
    hashdb::settings_t settings;
    error_message = hashdb::read_settings(hashdb_dir, settings);
    if (error_message.size() != 0) {
      return error_message;
    }

    // open scan manager
    hashdb::scan_manager_t scan_manager(hashdb_dir);

    // open the file reader
    const hasher::file_reader_t file_reader(hasher::utf8_to_native(
                                                           media_filename));
    if (file_reader.error_message.size() > 0) {
      // the file failed to open
      return file_reader.error_message;
    }

    // create the scan_tracker
    hasher::scan_tracker_t scan_tracker(file_reader.filesize);

    // get the number of CPUs
    const size_t num_cpus = hashdb::numCPU();

    // create the job queue to hold more jobs than threads
    hasher::job_queue_t* job_queue = new hasher::job_queue_t(num_cpus * 2);

    // create the threadpool that will process jobs until job_queue.is_done
    hasher::threadpool_t* const threadpool =
                               new hasher::threadpool_t(num_cpus, job_queue);

    // scan the file
    std::string success = scan_file(file_reader, scan_manager, scan_tracker,
                                    step_size, settings.block_size,
                                    process_embedded_data, scan_mode,
                                    job_queue);
    if (success.size() > 0) {
      std::stringstream ss;
      ss << "# Error while scanning file " << file_reader.filename
         << ", " << file_reader.error_message << "\n";
      hashdb::tprint(std::cout, ss.str());
    }

    // done
    job_queue->done_adding();
    delete threadpool;
    delete job_queue;

    std::cout << "# Total zero-byte blocks found: " << scan_tracker.zero_count
              << "\n";

    // success
    return "";
  }

} // end namespace hashdb

