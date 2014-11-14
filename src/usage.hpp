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

#ifndef USAGE_HPP
#define USAGE_HPP

#include <config.h>

// Standard includes
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <getopt.h>
#include "bloom_filter_manager.hpp"
#include "hashdb_settings.hpp"
#include "hash_t_selector.h"
#include "globals.hpp"

void usage() {
  hashdb_settings_t s;

  // print usage
  std::cout
  << "hashdb Version " << PACKAGE_VERSION  << "\n"
//  << "Usage: hashdb -h | -H | -v | -V | <command>\n"
  << "Usage: hashdb [-h|-H|--help|--Help] [-v|-V|--version] [-q|--quiet]\n"
  << "              [-f|--flags=<flags>] <command> [<args>]\n"
  << "\n"
  << "  -h, --help         print this message\n"
  << "  -H, --Help         print this message plus usage notes and examples\n"
  << "  -v, -V, --version, --Version    print version number\n"
  << "  -q, --quiet        quiet mode\n"
  << "  -f, --flags=flags  set B-Tree flags, any of: preload:cache_branches:\n"
  << "                     least_memory:low_memoy:balanced:fast:fastest\n"
  << "\n"
  << "hashdb supports the following commands:\n"
  << "\n"
  << "New database:\n"
  << "  create [options] <hashdb>\n"
  << "    Create a new <hashdb> hash database.\n"
  << "\n"
  << "    Options:\n"
  << "    -p, --hash_block_size=<hash block size>\n"
  << "      <hash block size>, in bytes, of hashes(default " << s.hash_block_size << ")\n"
  << "      expected <hash block size>, in bytes, or 0 for no restriction\n"
  << "      (default " << s.hash_block_size << ")\n"
  << "    -m, --max=<maximum>\n"
  << "      <maximum> number of hash duplicates allowed, or 0 for no limit\n"
  << "      (default " << s.maximum_hash_duplicates << ")\n"
  << "    --bloom <state>\n"
  << "      sets bloom filter <state> to enabled | disabled (default " << bloom_state_to_string(s.bloom1_is_used) << ")\n"
  << "    --bloom_n <n>\n"
  << "      expected total number <n> of distinct hashes (default " << bloom_filter_manager_t::approximate_M_to_n(s.bloom1_M_hash_size) << ")\n"
  << "    --bloom_kM <k:M>\n"
  << "      number of hash functions <k> and bits per hash <M> (default <k>=" << s.bloom1_k_hash_functions << "\n"
  << "      and <M>=" << s.bloom1_M_hash_size << " or <M>=value calculated from value in --bloom_n)\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>   the file path to the new hash database to create\n"
  << "\n"
  << "Import/Export:\n"
  << "  import [-r <repository name>] <hashdb> <DFXML file>\n"
  << "    Import hashes from file <DFXML file> into hash database <hashdb>.\n"
  << "\n"
  << "    Options:\n"
  << "    -r, --repository=<repository name>\n"
  << "      The repository name to use for the set of hashes being imported.\n"
  << "      (default is \"repository_\" followed by the <DFXML file> path).\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to insert the imported hashes into\n"
  << "    <DFXML file>   the DFXML file to import hashes from\n"
  << "\n"
  << "  import_tab [-r <repository name> [-s <sector size>] <hashdb> <tab file>\n"
  << "    Import hashes from file <tab file> into hash database <hashdb>.\n"
  << "\n"
  << "    Options:\n"
  << "    -r, --repository=<repository name>\n"
  << "      The repository name to use for the set of hashes being imported.\n"
  << "      (default is \"repository_\" followed by the <NIST file> path).\n"
  << "    -s, --sector_size=<sector size>\n"
  << "      The sector size associated with the hashes being imported.\n"
  << "      (default " << globals_t::default_import_tab_sector_size << ")\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to insert the imported hashes into\n"
  << "    <NIST file>   the NIST file to import hashes from\n"
  << "\n"
  << "  export <hashdb> <DFXML file>\n"
  << "    Export hashes from the <hashdb> to a <DFXML file>.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database containing hash values to be exported\n"
  << "    <DFXML file>   the new DFXML file to export hash values into\n"
  << "\n"
  << "Database manipulation:\n"
  << "  add <source hashdb> <destination hashdb>\n"
  << "    Copy hashes from the <source hashdb> to the <destination hashdb>.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb>       the source hash database to copy hashes from\n"
  << "    <destination hashdb>  the destination hash database to copy hashes into\n"
  << "\n"
  << "  add_multiple <source hashdb 1> <source hashdb 2> <destination hashdb>\n"
  << "    Perform a union add of <source hashdb 1> and <source hashdb 2>\n"
  << "    into the <destination hashdb>.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb 1>     a hash database to copy hashes from\n"
  << "    <source hashdb 2>     a second hash database to copy hashes from\n"
  << "    <destination hashdb>  the destination hash database to copy hashes into\n"
  << "\n"
  << "  add_repository <source hashdb> <destination hashdb> <repository name>\n"
  << "    Copy hashes from the <source hashdb> to the <destination hashdb>\n"
  << "    when the <repository name> matches.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb>       the source hash database to copy hashes from\n"
  << "    <destination hashdb>  the destination hash database to copy hashes into\n"
  << "    <repository name>     the repository name to match when adding hashes\n"
  << "\n"
  << "  intersect <source hashdb 1> <source hashdb 2> <destination hashdb>\n"
  << "    Copy hashes that are common to both <source hashdb 1> and\n"
  << "    <source hashdb 2> into <destination hashdb>.  Hashes and their sources\n"
  << "    must match.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb 1>     a hash databases to copy the intersection of\n"
  << "    <source hashdb 2>     a second hash databases to copy the intersection of\n"
  << "    <destination hashdb>  the destination hash database to copy the\n"
  << "                          intersection of exact matches into\n"
  << "\n"
  << "  intersect_hash <source hashdb 1> <source hashdb 2> <destination hashdb>\n"
  << "    Copy hashes that are common to both <source hashdb 1> and\n"
  << "    <source hashdb 2> into <destination hashdb>.  Hashes match when hash\n"
  << "    values match, even if their associated source repository name and\n"
  << "    filename do not match.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb 1>     a hash databases to copy the intersection of\n"
  << "    <source hashdb 2>     a second hash databases to copy the intersection of\n"
  << "    <destination hashdb>  the destination hash database to copy the\n"
  << "                          intersection of hashes into\n"
  << "\n"
  << "  subtract <source hashdb 1> <source hashdb 2> <destination hashdb>\n"
  << "    Copy hashes that are in <souce hashdb 1> and not in <source hashdb 2>\n"
  << "    into <destination hashdb>.  Hashes and their sources must match.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb 1>     the hash database containing hash values to be\n"
  << "                          added if they are not also in the other database\n"
  << "    <source hashdb 2>     the hash database containing the hash values that\n"
  << "                          will not be added\n"
  << "    <destination hashdb>  the hash database to add the difference of the\n"
  << "                          exact matches into\n"
  << "\n"
  << "  subtract_hash <source hashdb 1> <source hashdb 2> <destination hashdb>\n"
  << "    Copy hashes that are in <souce hashdb 1> and not in <source hashdb 2>\n"
  << "    into <destination hashdb>.  Hashes match when hash values match, even if\n"
  << "    their associated source repository name and filename do not match.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb 1>     the hash database containing hash values to be\n"
  << "                          added if they are not also in the other database\n"
  << "    <source hashdb 2>     the hash database containing the hash values that\n"
  << "                          will not be added\n"
  << "    <destination hashdb>  the hash database to add the difference of the\n"
  << "                          hashes into\n"
  << "\n"
  << "  deduplicate <source hashdb> <destination hashdb>\n"
  << "    Copy hashes in <source hashdb> into <destination hashdb> except\n"
  << "    for hashes defined multiple times.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <source hashdb>       the hash database to copy hashes from when source\n"
  << "                          hashes appear only once\n"
  << "    <destination hashdb>  the hash database to copy hashes to when source\n"
  << "                          hashes appear only once\n"
  << "\n"
  << "Scan services:\n"
  << "  scan <path_or_socket> <DFXML file>\n"
  << "    Scan the <path_or_socket> for hashes that match hashes in the\n"
  << "    <DFXML file> and print out matches.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>          the file path to the hash database to use as the\n"
  << "                      lookup source\n"
  << "    <DFXML file>      the DFXML file containing hashes to scan for\n"
  << "\n"
  << "  scan_hash <path_or_socket> <hash value>\n"
  << "    Scan the <path_or_socket> for the specified <hash value> and print\n"
  << "    out matches.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>          the file path to the hash database to use as the\n"
  << "                      lookup source\n"
  << "    <hash value>      the hash value to scan for\n"
  << "\n"
  << "  scan_expanded [-m <number>] <hashdb> <DFXML file>\n"
  << "    Scan the <hashdb> for hashes that match hashes in the <DFXML file>\n"
  << "    and print out matches showing all sources.  Specific source information\n"
  << "    is suppressed if the number of sources exceeds the requested maximum.\n"
  << "\n"
  << "    Options:\n"
  << "    -m <number>       <maximum> number of sources a hash can have before\n"
  << "                      suppressing printing them (default "
                   << globals_t::default_scan_expanded_max << ").\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>          the file path to the hash database to use as the\n"
  << "                      lookup source\n"
  << "    <DFXML file>      the DFXML file containing hashes to scan for\n"
  << "\n"
  << "  scan_expanded_hash <hashdb> <hash value>\n"
  << "    Scan the <hashdb> for the specified <hash value> and print out matches\n"
  << "    showing all sources.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>          the file path to the hash database to use as the\n"
  << "                      lookup source\n"
  << "    <hash value>      the hash value to scan for\n"
  << "\n"
  << "  server <hashdb> <port number>\n"
  << "    Start a query server service for <hashdb> at <port number> for servicing\n"
  << "    hashdb queries.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database that the server service will use\n"
  << "    <port number>  the TCP port to make available for clients, for\n"
  << "                   example '14500'\n"
  << "\n"
  << "Statistics:\n"
  << "  size <hashdb>\n"
  << "    Print out size information for the given <hashdb> database.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to print size information for\n"
  << "\n"
  << "  sources <hashdb>\n"
  << "    Print source information indicating where the hashes in the <hashdb>\n"
  << "    came from.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to print all the repository name,\n"
  << "                   filename source information for\n"
  << "\n"
  << "  histogram <hashdb>\n"
  << "    Print the histogram of hashes for the given <hashdb> database.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to print the histogram of hashes for\n"
  << "\n"
  << "  duplicates <hashdb> <number>\n"
  << "    Print the hashes in the given <hashdb> database that are sourced the\n"
  << "    given <number> of times.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to print duplicate hashes about\n"
  << "    <number>       the requested number of duplicate hashes\n"
  << "\n"
  << "  hash_table <hashdb> <repository name> <filename>\n"
  << "    Print the hashes and offsets from the given <hashdb> database for the\n"
  << "    <repository name> and <filename> source requested.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>              the hash database to print duplicate hashes for\n"
  << "    <repository name>     the repository name to match\n"
  << "    <filename >           the filename to match\n"
  << "\n"
  << "  expand_identified_blocks <hashdb> <identified blocks file>\n"
  << "    Print source information for each hash in <identified blocks file> by\n"
  << "    referencing source information in <hashdb>.  Source information\n"
  << "    includes repository name, filename, and file offset.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>                  the hash database to use as the lookup source\n"
  << "                              associated with the identified blocks file\n"
  << "    <identified_blocks.txt>   the identified blocks feature file generated\n"
  << "                              by bulk_extractor\n"
  << "\n"
  << "  explain_identified_blocks [-m <number>] <hashdb> <identified_blocks.txt>\n"
  << "    Print source information from the <hashdb> database for hashes in the\n"
  << "    <identified_blocks.txt> file for sources containing hashes that are not\n"
  << "    repeated more than a maximum number of times.\n"
  << "\n"
  << "    Options:\n"
  << "    -m <number>               <maximum> number of repeats allowed before\n"
  << "                              a hash is dropped (default "
                   << globals_t::default_explain_identified_blocks_max << ").\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>                  the hash database to use as the lookup source\n"
  << "                              associated with the identified blocks file\n"
  << "    <identified_blocks.txt>   the identified blocks feature file generated\n"
  << "                              by bulk_extractor\n"
  << "\n"
  << "Tuning:\n"
  << "  rebuild_bloom [options] <hashdb>\n"
  << "    Rebuild the bloom filter in the <hashdb> hash database.\n"
  << "\n"
  << "    Options:\n"
  << "    --bloom <state>\n"
  << "      sets bloom filter <state> to enabled | disabled (default " << bloom_state_to_string(s.bloom1_is_used) << ")\n"
  << "    --bloom_n <n>\n"
  << "      expected total number <n> of distinct hashes (default " << bloom_filter_manager_t::approximate_M_to_n(s.bloom1_M_hash_size) << ")\n"
  << "    --bloom_kM <k:M>\n"
  << "      number of hash functions <k> and bits per hash <M> (default <k>=" << s.bloom1_k_hash_functions << "\n"
  << "      and <M>=" << s.bloom1_M_hash_size << " or <M>=value calculated from value in --bloom_n)\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the  hash database for which the bloom filters will be\n"
  << "                   rebuilt\n"
  << "\n"
  << "  upgrade <hashdb>\n"
  << "    Make <hashdb> created by v1.0.0 compatible with hashdb v1.1.0+.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to upgrade\n"
  << "\n"
  << "Performance analysis:\n"
  << "  add_random [-r <repository name>] <hashdb> <count>\n"
  << "    Add <count> randomly generated hashes into hash database <hashdb>.\n"
  << "    Write performance data in the database's log.xml file.\n"
  << "\n"
  << "    Options:\n"
  << "    -r, --repository=<repository name>\n"
  << "      The repository name to use for the set of hashes being added.\n"
  << "      (default is \"repository_add_random\").\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to add randomly generated hashes into\n"
  << "    <count>        the number of randomly generated hashes to add\n"
  << "\n"
  << "  scan_random <hashdb> <hashdb copy>\n"
  << "    Scan for random hashes in the <hashdb> and <hashdb copy> databases.\n"
  << "    Writes performance data in the database's log.xml file.\n"
  << "\n"
  << "    Parameters:\n"
  << "    <hashdb>       the hash database to scan\n"
  << "    <hashdb copy>  a copy of the hash database to scan\n"
  << "\n"
  << "bulk_extractor hashdb scanner:\n"
  << "  bulk_extractor -e hashdb -S hashdb_mode=import -o outdir1 my_image1\n"
  << "    Imports hashes from my_image1 to outdir1/hashdb.hdb\n"
  << "\n"
  << "  bulk_extractor -e hashdb -S hashdb_mode=scan\n"
  << "                 -S hashdb_scan_path_or_socket=outdir1/hashdb.hdb\n"
  << "                 -o outdir2 my_image2\n"
  << "    Scans hashes from my_image2 against hashes in outdir1/hashdb.hdb\n"
  << "\n"
  ;
}

void detailed_usage() {
  hashdb_settings_t s;

  // print usage notes and examples
  std::cout
  << "Examples:\n"
  << "This example uses the md5deep tool to generate cryptographic hashes from\n"
  << "hash blocks in a file, and is suitable for importing into a hash database\n"
  << "using the hashdb \"import\" command.  Specifically:\n"
  << "\"-p 4096\" sets the hash block partition size to 4096 bytes.\n"
  << "\"-d\" instructs the md5deep tool to produce output in DFXML format.\n"
  << "\"my_file\" specifies the file that cryptographic hashes will be\n"
  << "generated for.\n"
  << "The output of md5deep is directed to file \"my_dfxml_file.xml\".\n"
  << "    md5deep -p 4096 -d my_file > my_dfxml_file.xml\n"
  << "\n"
  << "This example uses the md5deep tool to generate hashes recursively under\n"
  << "subdirectories, and is suitable for importing into a hash database using\n"
  << "the hashdb \"import\" command.  Specifically:\n"
  << "\"-p 4096\" sets the hash block partition size to 4096 bytes.\n"
  << "\"-d\" instructs the md5deep tool to produce output in DFXML format.\n"
  << "\"-r mydir\" specifies that hashes will be generated recursively under\n"
  << "directory mydir.\n"
  << "The output of md5deep is directed to file \"my_dfxml_file.xml\".\n"
  << "    md5deep -p 4096 -d -r my_dir > my_dfxml_file.xml\n"
  << "\n"
  << "This example creates a new hash database named my_hashdb.hdb with default\n"
  << "settings:\n"
  << "    hashdb create my_hashdb.hdb\n"
  << "\n"
  << "This example imports hashes into hash database my_hashdb.hdb from DFXML input\n"
  << "file my_dfxml_file.xml, categorizing the hashes as sourced from repository\n"
  << "\"my repository\":\n"
  << "    hashdb import -r \"my repository\" my_hashdb.hdb my_dfxml_file.xml\n"
  << "\n"
  << "This example exports hashes in my_hashdb.hdb to output DFXML file my_dfxml.xml:\n"
  << "    hashdb export my_hashdb my_dfxml.xml\n"
  << "\n"
  << "This example adds hashes from hash database my_hashdb1.hdb to hash database\n"
  << "my_hashdb2.hdb:\n"
  << "    hashdb add my_hashdb1.hdb my_hashdb2.hdb\n"
  << "\n"
  << "This example performs a database merge by adding my_hashdb1.hdb and my_hashdb2.hdb\n"
  << "into new hash database my_hashdb3.hdb:\n"
  << "    hashdb create my_hashdb3.hdb\n"
  << "    hashdb add_multiple my_hashdb1.hdb my_hashdb2.hdb my_hashdb3.hdb\n"
  << "\n"
  << "This example removes hashes in my_hashdb1.hdb from my_hashdb2.hdb:\n"
  << "    hashdb subtract my_hashdb1.hdb my_hashdb2.hdb\n"
  << "\n"
  << "This example creates a database without duplicates by copying all hashes\n"
  << "that appear only once in my_hashdb1.hdb into new database my_hashdb2.hdb:\n"
  << "    hashdb create my_hashdb2.hdb\n"
  << "    hashdb deduplicate my_hashdb1.hdb my_hashdb2.hdb\n"
  << "\n"
  << "This example rebuilds the Bloom filters for hash database my_hashdb.hdb to\n"
  << "optimize it to work well with 50,000,000 different hash values:\n"
  << "    hashdb rebuild_bloom --bloom_n 50000000 my_hashdb.hdb\n"
  << "\n"
  << "This example starts hashdb as a server service for the hash database at\n"
  << "path my_hashdb.hdb at port number \"14500\":\n"
  << "    hashdb server my_hashdb.hdb 14500\n"
  << "\n"
  << "This example searches the hashdb server service available at socket\n"
  << "tcp://localhost:14500 for hashes that match those in DFXML file my_dfxml.xml\n"
  << "and directs output to stdout:\n"
  << "    hashdb scan tcp://localhost:14500 my_dfxml.xml\n"
  << "\n"
  << "This example searches my_hashdb.hdb for hashes that match those in DFXML file\n"
  << "my_dfxml.xml and directs output to stdout:\n"
  << "    hashdb scan my_hashdb.hdb my_dfxml.xml\n"
  << "\n"
  << "This example searches my_hashdb.hdb for hashes that match MD5 hash value\n"
  << "d2d95... and directs output to stdout:\n"
  << "    hashdb scan_hash my_hashdb.hdb d2d958b44c481cc41b0121b3b4afae85\n"
  << "\n"
  << "This example prints out source metadata of where all hashes in my_hashdb.hdb\n"
  << "came from:\n"
  << "    hashdb sources my_hashdb.hdb\n"
  << "\n"
  << "This example prints out size information about the hash database at file\n"
  << "path my_hashdb.hdb:\n"
  << "    hashdb size my_hashdb.hdb\n"
  << "\n"
  << "This example prints out statistics about the hash database at file path\n"
  << "my_hashdb.hdb:\n"
  << "    hashdb statistics my_hashdb.hdb\n"
  << "\n"
  << "This example prints out duplicate hashes in my_hashdb.hdb that have been\n"
  << "sourced 20 times:\n"
  << "    hashdb duplicates my_hashdb.hdb 20\n"
  << "\n"
  << "This example prints out the table of hashes along with source information\n"
  << "for hashes in my_hashdb.hdb:\n"
  << "    hashdb hash_table my_hashdb.hdb\n"
  << "\n"
  << "This example uses bulk_extractor to scan for hash values in media image\n"
  << "my_image that match hashes in hash database my_hashdb.hdb, creating output in\n"
  << "feature file my_scan/identified_blocks.txt:\n"
  << "    bulk_extractor -e hashdb -S hashdb_mode=scan\n"
  << "    -S hashdb_scan_path_or_socket=my_hashdb.hdb -o my_scan my_image\n"
  << "\n"
  << "This example uses bulk_extractor to scan for hash values in the media image\n"
  << "available at socket tcp://localhost:14500, creating output in feature\n"
  << "file my_scan/identified_blocks.txt:\n"
  << "    bulk_extractor -e hashdb -S hashdb_mode=scan\n"
  << "    -S hashdb_scan_path_or_socket=tcp://localhost:14500 -o my_scan my_image\n"
  << "\n"
  << "This example uses bulk_extractor to import hash values from media image\n"
  << "my_image into hash database my_scan/hashdb.hdb:\n"
  << "    bulk_extractor -e hashdb -S hashdb_mode=import -o my_scan my_image\n"
  << "\n"
  << "This example creates new hash database my_hashdb.hdb using various tuning\n"
  << "parameters.  Specifically:\n"
  << "\"-p 512\" specifies that the hash database will contain hashes for data\n"
  << "hashed with a hash block size of 512 bytes.\n"
  << "\"-m 2\" specifies that when there are duplicate hashes, only the first\n"
  << "two hashes of a duplicate hash value will be copied.\n"
  << "\"--bloom enabled\" specifies that the Bloom filter is enabled.\n"
  << "\"--bloom_n 50000000\" specifies that the Bloom filter should be sized to expect\n"
  << "50,000,000 different hash values.\n"
  << "    hashdb create -p 512 -m 2 --bloom enabled --bloom_n 50000000\n"
  << "    my_hashdb.hdb\n"
  << "\n"
  << "Using the md5deep tool to generate hash data:\n"
  << "hashdb imports hashes from DFXML files that contain cryptographic\n"
  << "hashes of hash blocks.  These files can be generated using the md5deep tool\n"
  << "or by exporting a hash database using the hashdb \"export\" command.\n"
  << "When using the md5deep tool to generate hash data, the \"-p <partition size>\"\n"
  << "option must be set to the desired hash block size.  This value must match\n"
  << "the hash block size that hashdb expects or else no hashes will be\n"
  << "copied in.  The md5deep tool also requires the \"-d\" option in order to\n"
  << "instruct md5deep to generate output in DFXML format.  Please see the md5deep\n"
  << "man page.\n"
  << "\n"
  << "Using the bulk_extractor hashdb scanner:\n"
  << "The bulk_extractor hashdb scanner provides two capabilities: 1) scanning\n"
  << "a hash database for previously encountered hash values, and 2) importing\n"
  << "block hashes into a new hash database.  Options that control the hashdb\n"
  << "scanner are provided to bulk_extractor using \"-S name=value\" parameters\n"
  << "when bulk_extractor is invoked.  Please type \"bulk_extractor -h\" for\n"
  << "information on usage of the hashdb scanner.  Note that the hashdb scanner\n"
  << "is not available unless bulk_extractor has been compiled with hashdb support.\n"
  << "\n"
  << "Please see the hashdb Users Manual for further information.\n"
  ;
}

#endif

