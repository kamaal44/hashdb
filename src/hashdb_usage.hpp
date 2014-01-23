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
 * Provides usage and detailed usage for the hashdb tool.
 */

#ifndef HASHDB_USAGE_HPP
#define HASHDB_USAGE_HPP

#include <config.h>

// Standard includes
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <getopt.h>
#include "hashdb_settings.hpp"
#include "hashdb_runtime_options.hpp"

// approximate bloom conversions for k=3 and p false positive = ~ 1.1% to 6.4%
uint64_t approximate_M_to_n(uint32_t M) {
  uint64_t m = (uint64_t)1<<M;
  uint64_t n = m * 0.17;
//std::cout << "Bloom filter conversion: for M=" << M << " use n=" << n << "\n";
  return n;
}

// approximate bloom conversions for k=3 and p false positive = ~ 1.1% to 6.4%
uint32_t approximate_n_to_M(uint64_t n) {
  uint64_t m = n / 0.17;
  uint32_t M = 1;
  // fix with actual math formula, but this works
  while ((m = m/2) > 0) {
    M++;
  }
//std::cout << "Bloom filter conversion: for n=" << n << " use M=" << M << "\n";
  return M;
}

void usage() {
  hashdb_settings_t s;
  hashdb_runtime_options_t o;

  // print usage
  std::cout
  << "hashdb Version " << PACKAGE_VERSION  << "\n"
  << "Usage: hashdb -h | -H | -V | <command>\n"
  << "    -h, --help    print this message\n"
  << "    -H            print detailed help including usage notes and examples\n"
  << "    --Version     print version number\n"
  << "\n"
  << "hashdb supports the following <command> options:\n"
  << "\n"
  << "copy [<hashdb tuning parameter>]+ [-r <repository name>] <input> <hashdb>\n"
  << "    Copies the hashes in the <input> into the <hashdb> hash database.\n"
  << "\n"
  << "    Options:\n"
  << "    <hashdb tuning parameter>\n"
  << "        When a new <hashdb> hash database is being created,\n"
  << "        <hashdb tuning parameter> options may be provided to configure the\n"
  << "        hash database.  Please see <hashdb tuning parameter> options and\n"
  << "        <bloom filter tuning parameter> options for settings and default\n"
  << "        values.\n"
  << "\n"
  << "    -r, --repository=<repository name>\n"
  << "        When importing hashes from a md5deep generated DFXML <input> file,\n"
  << "        where a repository name is not specified, a <repository name> may\n"
  << "        be provided to speify the repository from which cryptographic hashes\n"
  << "        of hash blocks are sourced.  (default is \"repository_\" followed\n"
  << "        by the <DFXML file> path).\n"
  << "\n"
  << "    -x, --exclude_duplicates=<count>\n"
  << "        When copying hashes from an <input> hashdb hash dtatabase to a new\n"
  << "        <hashdb> hash database, do not copy any hashes that have <count>\n"
  << "        or more duplicates.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <input>    a md5deep generated DFXML file or another hashdb hash database\n"
  << "    <hashdb>   a hash database being created or a hash database being\n"
  << "               copied to\n"
  << "\n"
  << "remove [-r <repository name>] <input> <hashdb>\n"
  << "    Removes hashes in the <input> from the <hashdb> hash database.\n"
 << "\n"
  << "    Options:\n"
  << "    -r, --repository=<repository name>\n"
  << "        When removing hashes identified from a md5deep generated DFXML\n"
  << "        <input> file, where a repository name is not specified, a\n"
  << "        <repository name> may be provided to speify the repository from\n"
  << "        which cryptographic hashes of hash blocks will be removed.\n"
  << "        (default is \"repository_\"\n followed by the <DFXML file> path)\n"
  << "\n"
  << "    Parameters:\n"
  << "    <input>    a md5deep generated DFXML file or another hashdb hash database\n"
  << "    <hashdb>   a hash database in which hashes in the <input> will be\n"
  << "               removed\n"
  << "\n"
  << "merge [<hashdb tuning parameter>]+ <hashdb input 1> <hashdb input 2>\n"
  << "        <hashdb output>\n"
  << "    Merges hashes in the <hashdb input 1> and <hashdb input 2> databases\n"
  << "    into the new <hashdb output> database.\n"
  << "\n"
  << "    Options:\n"
  << "    <hashdb tuning parameter>\n"
  << "        When a new <hashdb> hash database is being created,\n"
  << "        <hashdb tuning parameter> options may be provided to configure the\n"
  << "        hash database.  Please see <hashdb tuning parameter> options and\n"
  << "        <bloom filter tuning parameter> options for settings and default\n"
  << "        values.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb input 1>    a hashdb hash database input\n"
  << "    <hashdb input 2>    a second hashdb hash database input\n"
  << "    <hashdb output>     a new hashdb hash database that will contain the\n"
  << "                        merged inputs\n"
  << "\n"
  << "rebuild_bloom [<bloom filter tuning parameter>]+ <hashdb>\n"
  << "    Rebuilds the bloom filters in the <hashdb> hash database.\n"
  << "\n"
  << "    Options:\n"
  << "    <bloom filter tuning parameter>\n"
  << "        Please see <bloom filter tuning parameter> options for settings\n"
  << "        and default values.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>    a hash database for which the bloom filters will be rebuilt\n"
  << "\n"
  << "export <hashdb> <DFXML file>\n"
  << "    Exports the hashes in the <hashdb> hash database to a new <DFXML file>.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb input>   a hash database whose hash values are to be exported\n"
  << "    <dfxml output>   a DFXML file containing the hashes in the <hashdb input>\n"
  << "\n"
  << "info <hashdb>\n"
  << "    Displays information about the <hashdb> hash database to stdout.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>         a hash database whose database information is to be\n"
  << "                     displayed\n"
  << "\n"
  << "server [-s] <server socket endpoint> <hashdb>\n"
  << "    Starts anager as a query server service for supporting hashdb\n"
  << "    queries.\n"
  << "\n"
  << "    Options:\n"
  << "    -s, --socket=<server path or socket>\n"
  << "        specifies the <server path or socket> to make available for clients.\n"
  << "        Valid paths are filesystem paths to valid hash databases.\n"
  << "        Valid sockets are specified by transport type.\n"
  << "        Valid socket transports supported by the zmq messaging kernel are\n"
  << "        tcp, ipc, and inproc.  Currently, only tcp is tested.\n"
  << "        (default '" << o.server_path << "')\n"
  << "\n"
  << "<hashdb tuning parameter> options set the configuration of a new hash\n"
  << "database:\n"
  << "    -p, --hash_block_size=<hash block size>\n"
  << "        <hash block size>, in bytes, used to generate hashes (default " << s.hash_block_size << ")\n"
  << "\n"
  << "    -m, --max_duplicates=<maximum>\n"
  << "        <maximum> number of hash duplicates allowed, or 0 for no limit\n"
  << "        (default " << s.maximum_hash_duplicates << ")\n"
  << "\n"
  << "    -t, --storage_type=<storage type>\n"
  << "        <storage type> to use in the hash database, where <storage type>\n"
  << "        is one of: btree | hash | red-black-tree | sorted-vector\n"
  << "        (default " << s.map_type << ")\n"
  << "\n"
  << "    -n, --shards=<number of shards>\n"
  << "        <number of shards> to use (default " << s.map_shard_count << ")\n"
  << "\n"
  << "    -i, --bits=<number of index bits>\n"
  << "        <number of index bits> to use for the source lookup index, between\n"
  << "        32 and 40 (default " << (uint32_t)s.number_of_index_bits << ")\n"
  << "        The number of bits used for the hash block offset value is\n"
  << "        (64 - <number of index bits>).\n"
  << "\n"
  << "<bloom filter tuning parameter> settings can help performance during hash\n"
  << "queries:\n"
  << "    --b1 <state>\n"
  << "        sets bloom filter 1 <state> to enabled | disabled (default " << bloom_state_to_string(s.bloom1_is_used) << ")\n"
  << "    --b1n <n>\n"
  << "        expected total number <n> of unique hashes (default " << approximate_M_to_n(s.bloom1_M_hash_size) << ")\n"
  << "    --b1kM <k:M>\n"
  << "        number of hash functions <k> and bits per hash <M> (default <k>=" << s.bloom1_k_hash_functions << "\n"
  << "        and <M>=" << s.bloom1_M_hash_size << " or <M>=value calculated from value in --b1n)\n"
  << "    --b2 <state>\n"
  << "        sets bloom filter 1 <state> to enabled | disabled (default " << bloom_state_to_string(s.bloom2_is_used) << ")\n"
  << "    --b2n <total>\n"
  << "        expected total number <n> of unique hashes (default " << approximate_M_to_n(s.bloom2_M_hash_size) << ")\n"
  << "    --b2kM <k:M>\n"
  << "        number of hash functions <k> and bits per hash <M> (default <k>=" << s.bloom2_k_hash_functions << "\n"
  << "        and <M>=" << s.bloom2_M_hash_size << " or <M>=value calculated from value in --b2n)\n"
  << "\n"
  ;
}

void detailed_usage() {
  hashdb_settings_t s;
  hashdb_runtime_options_t o;

  // print usage notes and examples
  std::cout
  << "Notes:\n"
  << "Using the md5deep tool to generate hash data:\n"
  << "hashdb imports hashes from DFXML files that contain cryptographic\n"
  << "hashes of hash blocks.  These files can be generated using the md5deep tool\n"
  << "or by exporting a hash database using the hashdb \"export\" command.\n"
  << "When using the md5deep tool to generate hash data, the \"-p <partition size>\"\n"
  << "option must be set to the desired hash block size.  This value must match\n"
  << "the hash block size that hashdb expects or else no hashes will be\n"
  << "copied in.  The md5deep tool also requires the \"-d\" option in order to\n"
  << "instruct md5deep to generate output in DFXML format.\n"
  << "\n"
  << "Selecting an optimal hash database storage type:\n"
  << "The storage type option, \"-t\", selects the storage type to use in the\n"
  << "hash database.  Each storage type has advantages and disadvantages:\n"
  << "    btree           Provides fast build times, fast access times, and is\n"
  << "                    fairly compact.\n"
  << "                    Currently, btree may have threading issues and may\n"
  << "                    crash when performing concurrent queries.\n"
  << "\n"
  << "    hash            Provides fastest query times and is very compact,\n"
  << "                    but is very slow during building.  We recommend\n"
  << "                    building a hash database using the btree storage type,\n"
  << "                    and, once built, copying it to a new hash database\n"
  << "                    using the hash storage type option.\n"
  << "\n"
  << "    red-black-tree  Similar in performance to btree, but not as fast or\n"
  << "                    compact.\n"
  << "\n"
  << "    sorted-vector   Similar in performance to hash.\n"
  << "\n"
  << "Improving query speed by using sharding:\n"
  << "Sharding splits hashes so that internal to the hash database, they are\n"
  << "distributed across multiple files.  The purpose of sharding is to reduce\n"
  << "the size of data structures and files.  It is not clear that sharding helps\n"
  << "performance by reducing the size of data structures.  Sharding does not\n"
  << "help performance by using multiple files because the files must all be\n"
  << "opened anyway.  In the future, when shards can be distributed across multiple\n"
  << "parallel processors, sharding can help performance significantly.\n"
  << "\n"
  << "Improving query speed by using Bloom filters:\n"
  << "Bloom filters can speed up performance during hash queries by quickly\n"
  << "indicating if a hash value is not in the hash database.  When the Bloom\n"
  << "filter indicates that a hash value is not in the hash database, an actual\n"
  << "hash database lookup is not required, and time is saved.  If the Bloom\n"
  << "filter indicates that the hash value may be in the hash database, a hash\n"
  << "database lookup is required and no time is saved.\n"
  << "\n"
  << "Bloom filters can be large and can take up lots of disk space and memory.\n"
  << "A Bloom filter with a false positive rate between 1\% and 10\% is effictive.\n"
  << "If the false-positive rate is low, the Bloom filter is unnecessarily large,\n"
  << "and it could be smaller.  If the false-positive rate is too high, there\n"
  << "will be so many false positives that hash database lookups will be required\n"
  << "anyway, defeating the value of the bloom filter.\n"
  << "\n"
  << "Up to two Bloom filters may be used.  The idea of using two is that the\n"
  << "first would be smaller and would thus be more likely to be fully cached\n"
  << "in memory.  If the first Bloom filter indicates that the hash may be present,\n"
  << "then the second bloom filter, which should be larger, is checked.  If the\n"
  << "second Bloom filter indicates that the hash may be present, then a hash\n"
  << "database lookup is required to be sure.\n"
  << "\n"
  << "Performing hash queries using the hashid scanner with bulk_extractor:\n"
  << "bulk_extractor may be used to scan the hash database for matching\n"
  << "cryptographic hashes if the hashid scanner is configured and enabled.\n"
  << "The hashid scanner runs either as a client with hashdb running as\n"
  << "a server to perform hash queries, or loads the hash database drectly and\n"
  << "performs queries directly.  The hashid scanner takes parameters from\n"
  << "bulk_extractor using bulk_extractor's \"-S name=value\" control parameter.\n"
  << " hashid accepts the following parameters:\n"
  << "\n"
  << "   -S query_type=use_path\n"
  << "      <query_type> used to perform the query, where <query_type>\n"
  << "      is one of use_path | use_socket (default use_path)\n"
  << "      use_path   - Lookups are performed from a hashdb in the filesystem\n"
  << "                   at the specified <path>.\n"
  << "      use_socket - Lookups are performed from a server service at the\n"
  << "                   specified <socket>.\n"
  << "   -S path=a valid hashdb directory path is required\n"
  << "      Specifies the <path> to the hash database to be used for performing\n"
  << "      the query service.  This option is only used when the query type\n"
  << "      is set to \"use_path\".\n"
  << "   -S socket=tcp://localhost:14500\n"
  << "      Specifies the client <socket> endpoint to use to connect with the\n"
  << "      hashdb server (default 'tcp://localhost:14500').  Valid socket\n"
  << "      transports supported by the zmq messaging kernel are tcp, ipc, and\n"
  << "      inproc.  Currently, only tcp is tested.  This opition is only valid\n"
  << "      when the query type is set to \"use_socket\".\n"
  << "   -S hash_block_size=4096    Hash block size, in bytes, used to generate\n"
  << "      cryptographic hashes\n"
  << "   -S sector_size=512    Sector size, in bytes\n"
  << "      Hashes are generated on each sector_size boundary.\n"
  << "\n"
  << "Performing hash queries using the hashdb_checker tool:\n"
  << "The hashdb_checker tool runs as a client service to scan a DFXML file for\n"
  << "cryptographic hash values that match values in a hash database. In order\n"
  << "to work, the hashdb_checker tool requires that the hashdb tool be\n"
  << "running as a server hash database query service at a matching socket\n"
  << "endpoint.  Please type \"hashdb_checker --help\" for more information on\n"
  << "the usage of the hashdb_checker tool.\n"
  << "\n"
  << "Improving startup speed by keeping a hash database open:\n"
  << "In the future, a dedicated provision may be created for this, but for now,\n"
  << "the time required to open a hash database may be avoided by keeping a\n"
  << "persistent hash database open by starting a hash database query server\n"
  << "service and keeping it running.  Now this hash database will open quickly\n"
  << "for other query services because it will already be cached in memory.\n"
  << "Caution, though, do not change the contents of a hash database that is\n"
  << "opened by multiple processes because this will make the copies inconsistent.\n"
  << "\n"
  << "Overloaded uses of the term \"hash\":\n"
  << "The term \"hash\" is overloaded and can mean any of the following:\n"
  << "   The MD5 hash value being recorded in the hash database.\n"
  << "   The hash storage type, specifically an unordered map,  used for storing\n"
  << "   information in the hash database.\n"
  << "   The hash that the hash storage type uses in order to map a MD5 hash\n"
  << "   record onto a hash storage slot.\n"
  << "   The hash that the Bloom filter uses to map onto a specific bit within\n"
  << "   the Bloom filter.\n"
  << "\n"
  << "Log files:\n"
  << "Commands that create or modify a hash database produce a log file in the\n"
  << "hash database directory called \"log.xml\".  Currently, the log file is\n"
  << "replaced each time.  In the future, log entries will append to existing\n"
  << "content.\n"
  << "\n"
  << "Known bugs:\n"
  << "Performing hash queries in a threaded environment using the btree storage\n"
  << "type causes intermittent crashes.  This was observed when running the\n"
  << "bulk_extractor hashid scanner when bulk_extractor was scanning recursive\n"
  << "directories.  This bug will be addressed in a future release of boost\n"
  << "btree.\n"
  << "\n"
  << "Examples:\n"
  << "This example uses the md5deep tool to generate cryptographic hashes from\n"
  << "hash blocks in a file, and is suitable for importing into a hash database\n"
  << "using the hashdb \"copy\" command.  Specifically:\n"
  << "\"-p 4096\" sets the hash block partition size to 4096 bytes.\n"
  << "\"-d\" instructs the md5deep tool to produce output in DFXML format.\n"
  << "\"my_file\" specifies the file that cryptographic hashes will be generated\n"
  << "for.\n"
  << "The output of md5deep is directed to file \"my_dfxml_file\".\n"
  << "    md5deep -p 4096 -d my_file > my_dfxml_file\n"
  << "\n"
  << "This example uses the md5deep tool to generate hashes recursively under\n"
  << "subdirectories, and is suitable for importing into a hash database using\n"
  << "the hashdb \"copy\" command.  Specifically:\n"
  << "\"-p 4096\" sets the hash block partition size to 4096 bytes.\n"
  << "\"-d\" instructs the md5deep tool to produce output in DFXML format.\n"
  << "\"-r mydir\" specifies that hashes will be generated recursively under\n"
  << "directory mydir.\n"
  << "The output of md5deep is directed to file \"my_dfxml_file\".\n"
  << "    md5deep -p 4096 -d -r my_dir > my_dfxml_file\n"
  << "\n"
  << "This example copies hashes from DFXML input file my_dfxml_file to new hash\n"
  << "database my_hashdb, categorizing the hashes as sourced from repository\n"
  << "\"my repository\":\n"
  << "    hashdb copy -r \"my repository\" my_dfxml_file my_hashdb\n"
  << "\n"
  << "This example copies hashes from hash database my_hashdb1 to hash database\n"
  << "my_hashdb2.  If my_hashdb2 does not exist, it will be created.  If\n"
  << "my_hashdb2 exists, hashes from my_hashdb1 will be added to it.\n"
  << "    hashdb copy my_hashdb1 my_hashdb2\n"
  << "\n"
  << "This example copies hashes from my_hashdb1 to new hash database my_hashdb2,\n"
  << "but uses \"-m 5\" to copy only the first five duplicate hashes of each\n"
  << "duplicate hash value:\n"
  << "    hashdb copy -m 5 my_hashdb1 my_hashdb2\n"
  << "\n"
  << "This example copies hashes from my_hashdb1 to new hash database my_hashdb2,\n"
  << "but uses \"-x 5\" to not copy any hashes from my_hashdb1 that have 5 or more\n"
  << "duplicates.\n"
  << "    hashdb copy -x 5 my_hashdb1 my_hashdb2\n"
  << "\n"
  << "This example copies hashes from my_hashdb1 to new hash database my_hashdb2\n"
  << "using various tuning parameters.  Specifically:\n"
  << "\"-p 512\" specifies that the hash database will contain hashes for data\n"
  << "hashed with a hash block size of 512 bytes.\n"
  << "\"-m 2\" specifies that when there are duplicate hashes, only the first\n"
  << "two hashes of a duplicate hash value will be copied.\n"
  << "\"-t hash\" specifies that hashes will be recorded using the \"hash\" storage\n"
  << "type algorithm.\n"
  << "\"-n 4\" specifies that, internal to the hash database, hash values will be\n"
  << "sharded across four files.\n"
  << "\"-i 34\" specifies that 34 bits are allocated for the source lookup index,\n"
  << "allowing 2^34 entries of source lookup data.  Note that this leaves 2^30\n"
  << "entries remaining for hash block offset values.\n"
  << "\"--b1 enabled\" specifies that Bloom filter 1 is enabled.\n"
  << "\"--b1n 50000000\" specifies that Bloom filter 1 should be sized to expect\n"
  << "50,000,000 different hash values.\n"
  << "\"--b2 enabled\" specifies that Bloom filter 2 is enabled.\n"
  << "\"--b2kM 4:32 enabled\" specifies that Bloom filter 2 will be configured to\n"
  << "have 4 hash functions and that the Bloom filter hash function size will be\n"
  << "32 bits, consuming 512MiB of disk space.\n"
  << "    hashdb copy -p 512 -m 2 -t hash -n 4 -i 34 --b1 enabled\n"
  << "                --b1n 50000000 --b2 enabled --b2kM 4:32 my_hashdb1 my_hashdb2\n"
  << "\n"
  << "This example removes hashes in my_dfxml_file from my_hashdb using a DFXML\n"
  << "repository source name of \"my repository\":\n"
  << "    hashdb remove -r \"my repository\" my_dfxml_file my_hashdb\n"
  << "\n"
  << "This example merges my_hashdb1 and my_hashdb2 into new hash database\n"
  << "my_hashdb3:\n"
  << "    hashdb merge my_hashdb1 my_hashdb2 my_hashdb3\n"
  << "\n"
  << "This example rebuilds the Bloom filters for hash database my_hashdb to\n"
  << "optimize it to work well with 50,000,000 different hash values:\n"
  << "    hashdb rebuild_bloom --b1n 50000000 my_hashdb\n"
  << "\n"
  << "This example exports hashes in my_hashdb to new DFXML file my_dfxml:\n"
  << "    hashdb export my_hashdb my_dfxml\n"
  << "\n"
  << "This example displays the history attribution log of hash database my_hashdb.\n"
  << "Output is directed to stdout.\n"
  << "    hashdb info my_hashdb\n"
  << "\n"
  << "This example starts hashdb as a server service using socket endpoint\n"
  << "\"tcp://*:14501\".  It provides hash lookups using hash database my_hashdb:\n"
  << "    hashdb server -s tcp://*:14501 my_hashdb\n"
  << "\n"
  << "This example uses bulk_extractor to run the hashid scanner to scan for\n"
  << "hash values in a media file where the hash queries are performed\n"
  << "locally from a hashdb database that is opened by the hashid scanner.\n"
  << "Parameters to bulk_extractor for this example follow:\n"
  << "\"-S query_type=use_path\" tells the scanner to perform hash queries\n"
  << "using a hashdb at a local file path.\n"
  << "\"-S path=my_hashdb\" tells the scanner to perform hash queries\n"
  << "using local hashdb my_hashdb.\n"
  << "\"-S hash_block_size=4096\" tells the scanner to create cryptographic hashes\n"
  << "on 4096-byte chunks of data.\n"
  << "\"-S sector_size=512\" tells the scanner to create cryptographic hashes at\n"
  << "every 512-byte sector boundary.\n"
  << "\"-o scanner_output\" tells bulk_extractor to put scanner output into the\n"
  << "scanner_output directory.\n"
  << "File \"my_imagefile\" is the name of the image file that the scanner will use.\n"
  << "Specifically, the scanner will create hashes from hash blocks at each\n"
  << "sector boundary.\n"
  << "    bulk_extractor -S query_type=use_path\n"
  << "                   -S path=my_hashdb\n"
  << "                   -S hash_block_size=4096\n"
  << "                   -S sector_size=512\n"
  << "                   -o scanner_output my_imagefile\n"
  << "\n"
  << "This example uses bulk_extractor to run the scan_hashid scanner to scan\n"
  << "for hash values in a media file where the hash queries are performed\n"
  << "remotely using a hash database query server service available at a socket\n"
  << "endpoint.  Parameters to bulk_extractor for this example follow:\n"
  << "\"-S query_type=use_socket\" tells the scanner to perform hash queries\n"
  << "using a query server at a socket endpoint.\n"
  << "\"-S socket=tcp://localhost:14501\" sets the socket so that queries use a\n"
  << "hashdb query server at socket endpoint \"tcp://localhost:14501\".\n"
  << "hashdb must be running and available at\n"
  << "socket endpoint \"tcp://*:14501\" or else this example will fail because\n"
  << "a server service is not available.  Please see the example for starting\n"
  << "hashdb as a server query service.\n"
  << "\"-S hash_block_size=4096\" tells the scanner to create cryptographic\n"
  << "hashes on 4096-byte chunks of data.\n"
  << "\"-S sector_size=512\" tells the scanner to create cryptographic hashes at\n"
  << "every 512-byte sector boundary.\n"
  << "\"-o scanner_output\" tells bulk_extractor to put scanner output into the\n"
  << "scanner_output directory.\n"
  << "File \"my_imagefile\" is the name of the image file that the scanner will use.\n"
  << "Specifically, the scanner will create hashes from hash blocks at each\n"
  << "sector boundary.\n"
  << "    bulk_extractor -S query_type=use_socket\n"
  << "                   -S socket=tcp://localhost:14501\n"
  << "                   -S hash_block_size=4096\n"
  << "                   -S sector_size=512\n"
  << "                   -o scanner_output my_imagefile\n"
  << "\n"
  << "This example uses the hashdb_checker tool to determine if hash values in\n"
  << "file my_dfxml match hash values in the hashdb that is opened locally for\n"
  << "querying from.\n"
  << "Parameters to the hashdb_checker tool follow:\n"
  << "\"query_hash\" tells hashdb_checker to perform a hash query.\n"
  << "\"-q use_socket\" directs the query to use a hash database query server.\n"
  << "service for performing the hash lookup.\n"
  << "\"-s tcp://localhost:14501\" specifies the client socket endpoint as\n"
  << "\"tcp://localhost:14501\".  hashdb must be running and available\n"
  << "at socket endpoint \"tcp://*:14501\" or else this example will fail\n"
  << "because a server service is not available.  Please see the example for\n"
  << "starting hashdb as a server query service.\n"
  << "File \"my_dfxml\" is the name of the DFXML file containing hashes that will\n"
  << "be scanned for.\n"
  << "Output is directed to stdout.\n"
  << "    hashdb_checker query_hash -q use_socket -s tcp://localhost:14501 my_dfxml\n"
  << "\n"
  << "This example uses the hashdb_checker tool to look up source information\n"
  << "in feature file \"identified_blocks.txt\" created by the hashid scanner\n"
  << "while running bulk_extractor.\n"
  << "Parameters to the hashdb_checker tool follow:\n"
  << "\"query_source\" tells hashdb_checker to perform a source lookup query.\n"
  << "\"-q use_path\" directs the query to perform the queries using a path to\n"
  << "a hashdb resident in the local filesystem.\n"
  << "\"-p my_hashdb\" specifies \"my_hshdb\" as the file path to the hash database.\n"
  << "\"identified_blocks.txt\" is the feature file containing the hash values\n"
  << "to look up source information for.\n"
  << "Output is directed to stdout.\n"
  << "    hashdb_checker query_source -q use_path -p my_hashdb identified_blocks.txt\n"
  << "\n"
  << "This example uses the hashdb_checker tool to display information about\n"
  << "the hashdb being used by a server query service.\n"
  << "Parameters to the hashdb_checker tool follow:\n"
  << "\"query_hashdb_info\" tells hashdb_checker to return information about\n"
  << "the hashdb that it is using.\n"
  << "\"-q use_socket\" directs the query to use a hash database query server.\n"
  << "\"-s tcp://localhost:14501\" specifies the client socket endpoint as\n"
  << "\"tcp://localhost:14501\".  hashdb must be running and available\n"
  << "at socket endpoint \"tcp://*:14501\" or else this example will fail\n"
  << "because a server service is not available.  Please see the example for\n"
  << "starting hashdb as a server query service.\n"
  << "Output is directed to stdout.\n"
  << "    hashdb_checker query_hashdb_info -q use_socket -s tcp://localhost:14501\n"
  << "\n"
  ;
}

#endif
