#ifndef _DB_ROCKSDB_READ_ONLY_H_
#define _DB_ROCKSDB_READ_ONLY_H_

#include <mutex>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/status.h>
#include <unordered_map>

class RocksDBReadOnly
{
public:
    RocksDBReadOnly(const std::string &db_path, rocksdb::Status &status);
    ~RocksDBReadOnly();

    bool GetAllDataByColumnFamily(const std::string &column_family_name, std::unordered_map<std::string, std::string> &data, rocksdb::Status &status);
    bool MultiReadData(const std::vector<rocksdb::Slice> &keys, std::vector<std::string> &values, std::vector<rocksdb::Status> &status);
    bool ReadData(const std::string &column_family_name, const std::string &key,
                  std::string &value, rocksdb::Status &status);

private:
    RocksDBReadOnly(RocksDBReadOnly &&) = delete;
    RocksDBReadOnly(const RocksDBReadOnly &) = delete;
    RocksDBReadOnly &operator=(RocksDBReadOnly &&) = delete;
    RocksDBReadOnly &operator=(const RocksDBReadOnly &) = delete;

    bool init_success_;
    rocksdb::ReadOptions read_options_;
    rocksdb::DB *db_;
    std::vector<rocksdb::ColumnFamilyHandle *> cf_handles_;
    std::map<std::string, rocksdb::ColumnFamilyHandle *> column_family_handles_;
};

#endif
