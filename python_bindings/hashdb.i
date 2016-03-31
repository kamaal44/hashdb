%module (docstring="The hashdb module provides interfaces to the hashdb block hash database.") hashdb
%include "std_string.i"
%include "stdint.i"
%include "std_set.i"
%include "std_pair.i"

%{
#include "hashdb.hpp"
%}

%feature("autodoc", "1");

namespace hashdb {

  // ************************************************************
  // version of the hashdb library
  // ************************************************************
  /**
   * Version of the hashdb library.
   */
  extern "C"
  const char* version();

  // ************************************************************
  // settings
  // ************************************************************
  /**
   * Provides hashdb settings.
   *
   * Attributes:
   *   settings_version - The version of the settings record
   *   sector_size - Minimal sector size of data, in bytes.  Blocks must
   *     align to this.
   *   block_size - Size, in bytes, of data blocks.
   *   max_source_offset_pairs - The maximum number of source hash,
   *     file offset pairs to store for a single hash value.
   *   hash_prefix_bits - The number of hash prefix bits to use as the
   *     key in the optimized hash storage.
   *   hash_suffix_bytes - The number of hash suffix bytes to use as the
   *     value in the optimized hash storage.
   */
  struct settings_t {
//    static const uint32_t CURRENT_SETTINGS_VERSION = 3;
    uint32_t settings_version;
    uint32_t sector_size;
    uint32_t block_size;
    uint32_t max_source_offset_pairs;
    uint32_t hash_prefix_bits;
    uint32_t hash_suffix_bytes;
    settings_t();
    std::string settings_string() const;
  };

  // ************************************************************
  // misc support interfaces
  // ************************************************************
  /**
   * Create a new hashdb.
   * Return true and "" if hashdb is created, false and reason if not.
   * The current implementation may abort if something worse than a simple
   * path problem happens.
   *
   * Parameters:
   *   hashdb_dir - Path to the database to create.  The path must not
   *     exist yet.
   *   settings - The hashdb settings.
   *   command_string - String to put into the new hashdb log.
   *
   * Returns:
   *   "" if successful else reason if not.
   */
  std::string create_hashdb(const std::string& hashdb_dir,
                            const hashdb::settings_t& settings,
                            const std::string& command_string);

  /**
   * Return hashdb settings else reason for failure.
   * The current implementation may abort if something worse than a simple
   * path problem happens.
   *
   * Parameters:
   *   hashdb_dir - Path to the database to obtain the settings of.
   *   settings - The hashdb settings.
   *
   * Returns:
   *   True and "" if settings were retrieved, false and reason if not.
   */
  std::string read_settings(const std::string& hashdb_dir,
                            hashdb::settings_t& OUTPUT      // settings
                           );

//  /**
//   * Print environment information to the stream.  Specifically, print
//   * lines starting with the pound character followed by version information,
//   * the command line, the username, if available, and the date.
//   */
//  void print_environment(const std::string& command_line, std::ostream& os);

  /**
   * Return binary string or empty if hexdigest length is not even
   * or has any invalid digits.
   */
  std::string hex_to_bin(const std::string& hex_string);

  /**
   * Return hexadecimal representation of the binary string.
   */
  std::string bin_to_hex(const std::string& binary_hash);

  // ************************************************************
  // import
  // ************************************************************
  /**
   * Manage all LMDB updates.  All interfaces are locked and threadsafe.
   * A logger is opened for logging the command and for logging
   * timestamps and changes applied during the session.  Upon closure,
   * changes are written to the logger and the logger is closed.
   */
  class import_manager_t {

    private:
    lmdb_hash_data_manager_t* lmdb_hash_data_manager;
    lmdb_hash_manager_t* lmdb_hash_manager;
    lmdb_source_data_manager_t* lmdb_source_data_manager;
    lmdb_source_id_manager_t* lmdb_source_id_manager;
    lmdb_source_name_manager_t* lmdb_source_name_manager;

    logger_t* logger;
    hashdb::lmdb_changes_t* changes;

    public:
    // do not allow copy or assignment

    /**
     * Open hashdb for importing.
     *
     * Parameters:
     *   hashdb_dir - Path to the hashdb data store to import into.
     *   command_string - String to put into the new hashdb log.
     */
    import_manager_t(const std::string& hashdb_dir,
                     const std::string& command_string);

    /**
     * The destructor closes the log file and data store resources.
     */
    ~import_manager_t();

    /**
     * Insert the repository_name, filename pair associated with the
     * source.
     */
    void insert_source_name(const std::string& file_binary_hash,
                            const std::string& repository_name,
                            const std::string& filename);


    /**
     * Insert or change source data.
     *
     * Parameters:
     *   file_binary_hash - The MD5 hash of the source in binary form.
     *   filesize - The size of the source, in bytes.
     *   file_type - A string representing the type of the file.
     *   nonprobative_count - The count of non-probative hashes
     *     identified for this source.
     */
    void insert_source_data(const std::string& file_binary_hash,
                            const uint64_t filesize,
                            const std::string& file_type,
                            const uint64_t nonprobative_count);

    /**
     * Insert or change the hash data associated with the binary_hash.
     *
     * Parameters:
     *   binary_hash - The block hash in binary form.
     *   file_binary_hash - The MD5 hash of the source in binary form.
     *   file_offset - The byte offset into the file where the hash is
     *     located.
     *   entropy - A numeric entropy value for the associated block.
     *   block_label - Text indicating the type of the block or "" for
     *     no label.
     */
    void insert_hash(const std::string& binary_hash,
                     const std::string& file_binary_hash,
                     const uint64_t file_offset,
                     const uint64_t entropy,
                     const std::string& block_label);

    /**
     * Insert hash or source information from JSON record.
     *
     * Parameters:
     *   json_string - Hash or source text in JSON format.
     *
     *   Example hash syntax:
     *     {
     *       "block_hash": "a7df...",
     *       "entropy": 8,
     *       "block_label": "W",
     *       "source_offset_pairs": ["b9e7...", 4096]
     *     }
     *
     *   Example source syntax:
     *     {
     *       "file_hash": "b9e7...",
     *       "filesize": 8000,
     *       "file_type": "exe",
     *       "nonprobative_count": 4,
     *       "name_pairs": ["repository1", "filename1", "repo2", "f2"]
     *     }
     *
     * Returns:
     *   "" else error message if JSON is invalid.
     */
    std::string insert_json(const std::string& json_string);

    /**
     * Returns sizes of LMDB databases in the data store.
     */
    std::string sizes() const;
  };

  // ************************************************************
  // scan
  // ************************************************************
  /**
   * Manage LMDB scans.  Interfaces should be threadsafe by LMDB design.
   */
  class scan_manager_t {

    private:
    lmdb_hash_data_manager_t* lmdb_hash_data_manager;
    lmdb_hash_manager_t* lmdb_hash_manager;
    lmdb_source_data_manager_t* lmdb_source_data_manager;
    lmdb_source_id_manager_t* lmdb_source_id_manager;
    lmdb_source_name_manager_t* lmdb_source_name_manager;

    // support scan_expanded
    std::set<std::string>* hashes;
    std::set<std::string>* sources;

    public:
    // do not allow copy or assignment

    /**
     * Open hashdb for scanning.
     *
     * Parameters:
     *   hashdb_dir - Path to the database to scan against.
     */
    scan_manager_t(const std::string& hashdb_dir);

    /**
     * The destructor closes read-only data store resources.
     */
    ~scan_manager_t();

    /**
     * Scan for a hash and return expanded source information associated
     * with it.
     *
     * scan_manager caches hashes and source IDs and does not return
     * source information for hashes or sources that has already been
     * returned.
     *
     * Parameters:
     *   binary_hash - The block hash in binary form to scan for.
     *   expanded_text - Text about matched sources, or blank if text
     *     for the scanned hash has been returned in a previous scan.
     *
     *     Text is in JSON format.  Example syntax:
     * 
     * {
     *   "entropy": 8,
     *   "block_label": "W",
     *   "source_list_id": 57,
     *   "sources": [{
     *     "file_hash": "f7035a...",
     *     "filesize": 800,
     *     "file_type": "exe",
     *     "nonprobative_count": 2,
     *     "names": ["repository1", "filename1", "repo2", "f2"]
     *   }],
     *   "source_offset_pairs": ["f7035a...", 0, "f7035a...", 512]
     * }
     *
     * Returns:
     *   True if the hash is present, false if not.
     */
    std::string find_expanded_hash(const std::string& binary_hash);

    /**
     * Find hash, return JSON string else "" if not there.
     *
     * Parameters:
     *   binary_hash - The block hash in binary form.
     *   json_hash_string - Hash text in JSON format.  Example syntax:
     *
     *    {
     *      "block_hash": "a7df...",
     *      "entropy": 8,
     *      "block_label": "W",
     *      "source_offset_pairs": ["b9e7...", 4096]
     *    }
     *
     * Returns:
     *   JSON text if hash is present, false and "" if not.
     */
    std::string find_hash_json(const std::string& binary_hash) const;

    /**
     * Find source, return JSON string else "" if not there.
     *
     * Parameters:
     *   file_binary_hash - The file hash in binary form.
     *   json_source_string - Source text in JSON format.  Example syntax:
     *
     *     {
     *       "file_hash": "b9e7...",
     *       "filesize": 8000,
     *       "file_type": "exe",
     *       "nonprobative_count": 4,
     *       "name_pairs": ["repository1", "filename1", "repo2", "f2"]
     *       }
     *
     * Returns:
     *   JSON text if source is present, false and "" if not.
     */
    std::string find_source_json(const std::string& json_source_string) const;

    /**
     * Find hash count.  Faster than find_hash.  Accesses the hash
     * information store.
     *
     * Parameters:
     *   binary_hash - The block hash in binary form.
     *
     * Returns:
     *   Approximate hash count.
     */
    size_t find_hash_count(const std::string& binary_hash) const;

    /**
     * Find approximate hash count.  Faster than find_hash, but can be wrong.
     * Accesses the hash store.
     *
     * Parameters:
     *   binary_hash - The block hash in binary form.
     *
     * Returns:
     *   Approximate hash count.
     */
    size_t find_approximate_hash_count(const std::string& binary_hash) const;

    /**
     * Find source data for the given source ID, false on no source ID.
     *
     * Parameters:
     *   file_binary_hash - The MD5 hash of the source in binary form.
     *   filesize - The size of the source, in bytes.
     *   file_type - A string representing the type of the file.
     *   nonprobative_count - The count of non-probative hashes
     *     identified for this source.
     */
    bool find_source_data(const std::string& file_binary_hash,
                          uint64_t& filesize,
                          std::string& file_type,
                          uint64_t& nonprobative_count) const;

    /**
     * Return the first block hash in the database.
     *
     * Returns:
     *   binary_hash if a first hash is available else "" if DB is empty.
     */
    std::string first_hash() const;

    /**
     * Return the next block hash in the database.  Error if last hash
     *   does not exist.
     *
     * Parameters:
     *   last_binary_hash - The previous block hash in binary form.
     *
     * Returns:
     *   binary_hash if a next hash is available else "" if at end.
     */
    std::string next_hash(const std::string& binary_hash) const;

    /**
     * Return the file_binary_hash of the first source in the database.
     *
     * Returns:
     *   file_binary_hash if a first source is available else "" if DB
     *   is empty.
     */
    std::string first_source() const;

    /**
     * Return the next source in the database.  Error if last_file_binary_hash
     *   does not exist.
     *
     * Parameters:
     *   last_file_binary_hash - The previous source file hash in binary form.
     *
     * Returns:
     *   file_binary_hash if a next source is available else "" if at end.
     */
    std::string next_source(const std::string& file_binary_hash) const;

    /**
     * Return sizes of LMDB databases in JSON format.
     */
    std::string sizes() const;

    /**
     * Return the number of hashes.
     */
    size_t size_hashes() const;

    /**
     * Return the number of sources.
     */
    size_t size_sources() const;
  };

  // ************************************************************
  // timestamp
  // ************************************************************
  /**
   * Provide a timestamp service.
   */
  class timestamp_t {

    private:
    struct timeval* t0;
    struct timeval* t_last_timestamp;

    public:

    /**
     * Create a timestamp service.
     */
    timestamp_t();

    /**
     * Release timestamp resources.
     */
    ~timestamp_t();

    // do not allow copy or assignment

    /**
     * Create a named timestamp and return a JSON string in format
     * {"name":"name", "delta":delta, "total":total}.
     */
    std::string stamp(const std::string &name);
  };
} // end namespace hashdb