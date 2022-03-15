#include "rocksdb_read_only.h"
#include "log/logging.h"

RocksDBReadOnly::RocksDBReadOnly(const std::string &db_path, rocksdb::Status &status)
{
    init_success_ = false;
    std::vector<std::string> column_family_names;
    status = rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(), db_path, &column_family_names);
    if (!status.ok())
    {
        ERRORLOG("rocksdb ListColumnFamilies failed code:({}),subcode:({}),severity:({}),info:({})", status.code(), status.subcode(), status.severity(), status.ToString());
        return;
    }
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    for (auto &name : column_family_names)
    {
        column_families.push_back(rocksdb::ColumnFamilyDescriptor(name, rocksdb::ColumnFamilyOptions()));
    }

    db_ = nullptr;
    status = rocksdb::DB::OpenForReadOnly(rocksdb::Options(), db_path, column_families, &cf_handles_, &db_);
    if (!status.ok())
    {
        ERRORLOG("rocksdb OpenForReadOnly failed code:({}),subcode:({}),severity:({}),info:({})", status.code(), status.subcode(), status.severity(), status.ToString());
        return;
    }
    for (auto &handle : cf_handles_)
    {
        column_family_handles_.insert(std::make_pair(handle->GetName(), handle));
    }
    init_success_ = true;
}

RocksDBReadOnly::~RocksDBReadOnly()
{
    init_success_ = false;
    for (auto &handle : cf_handles_)
    {
        delete handle;
        handle = nullptr;
    }
    for (auto &handle : column_family_handles_)
    {
        handle.second = nullptr;
    }
    delete db_;
    db_ = nullptr;
}

bool RocksDBReadOnly::GetAllDataByColumnFamily(const std::string &column_family_name,
                                               std::unordered_map<std::string, std::string> &data,
                                               rocksdb::Status &status)
{
if (!init_success_)
    {
        ERRORLOG("Rocksdb Uninitialized");
        return false;
    }
    auto iter = column_family_handles_.find(column_family_name);
    if (column_family_handles_.end() == iter)
    {
        ERRORLOG("column family not found");
        return false;
    }

    rocksdb::Iterator *it = db_->NewIterator(rocksdb::ReadOptions(), column_family_handles_.at(column_family_name));
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        data.insert(std::make_pair(it->key().ToString(), it->value().ToString()));
    }
    bool flag = true;
    if (!(it->status().ok()))
    {
        ERRORLOG("Iterator:{}", it->status().ToString());
        flag = false;
    }
    delete it;
    it = nullptr;
    return true;
}

bool RocksDBReadOnly::MultiReadData(const std::vector<rocksdb::Slice> &keys, std::vector<std::string> &values, std::vector<rocksdb::Status> &statuses)
{
    if (!init_success_)
    {
        ERRORLOG("Rocksdb Uninitialized");
        return false;
    }
    statuses = db_->MultiGet(read_options_, cf_handles_, keys, &values);
    bool flag = true;
    for (size_t i = 0; i < statuses.size(); ++i)
    {
        auto status = statuses.at(i);
        if (!status.ok())
        {
            flag = false;
            std::string key;
            if (keys.size() > i)
            {
                key = keys.at(i).data();
            }
            if (status.IsNotFound())
            {
                TRACELOG("rocksdb ReadData failed key:{} code:({}),subcode:({}),severity:({}),info:({})", key, status.code(), status.subcode(), status.severity(), status.ToString());
            }
            else
            {
                ERRORLOG("rocksdb ReadData failed key:{} code:({}),subcode:({}),severity:({}),info:({})", key, status.code(), status.subcode(), status.severity(), status.ToString());
            }
        }
    }
    return flag;
}

bool RocksDBReadOnly::ReadData(const std::string &column_family_name, const std::string &key, std::string &value, rocksdb::Status &status)
{
    if (!init_success_)
    {
        ERRORLOG("Rocksdb Uninitialized");
        return false;
    }
    auto it = column_family_handles_.find(column_family_name);
    if (column_family_handles_.end() == it)
    {
        ERRORLOG("column family not found");
        return false;
    }
    status = db_->Get(read_options_, column_family_handles_.at(column_family_name), key, &value);
    if (status.ok())
    {
        return true;
    }
    if (status.IsNotFound())
    {
        TRACELOG("rocksdb ReadData failed key:{} code:({}),subcode:({}),severity:({}),info:({})", key, status.code(), status.subcode(), status.severity(), status.ToString());
    }
    else
    {
        ERRORLOG("rocksdb ReadData failed key:{} code:({}),subcode:({}),severity:({}),info:({})", key, status.code(), status.subcode(), status.severity(), status.ToString());
    }
    return false;
}
