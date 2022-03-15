#include "db/rocksdb_read_only.h"
#include "utils/crypto_utils.h"
#include <endian.h>
#include <fstream>
#include <iostream>
#include "molecule/blockchain.h"

std::string COLUMN_INDEX = "0";              // 链索引
std::string COLUMN_BLOCK_HEADER = "1";       // 块头
std::string COLUMN_BLOCK_BODY = "2";         // 交易体
std::string COLUMN_BLOCK_UNCLE = "3";        // store block's uncle and uncles’ proposal zones
std::string COLUMN_META = "4";               // store meta data
std::string COLUMN_TRANSACTION_INFO = "5"; // 交易扩展信息
std::string COLUMN_BLOCK_EXT = "6";        // 区块扩展信息
std::string COLUMN_BLOCK_PROPOSAL_IDS = "7"; // store block's proposal ids
std::string COLUMN_BLOCK_EPOCH = "8"; // store indicates track block epoch
std::string COLUMN_EPOCH = "9";       // store indicates track block epoch
std::string COLUMN_CELL = "10";       // store cell
std::string COLUMN_UNCLES = "11";     // store main chain consensus include uncles
std::string COLUMN_CELL_DATA = "12";  // store cell data
std::string COLUMN_NUMBER_HASH = "13";       // store block number-hash pair
std::string COLUMN_CELL_DATA_HASH = "14"; // store cell data hash
std::string COLUMN_BLOCK_EXTENSION = "15";   // 区块扩展数据

std::string db_path("/home/shaorongqiang/blockchain/ckb/target/debug/node/data/db");

int main_uncles(int argc, char **argv)
{
    rocksdb::Status status;
    RocksDBReadOnly db(db_path, status);
    if (!status.ok())
    {
        return -1;
    }
    std::unordered_map<std::string, std::string> datas;
    if (!db.GetAllDataByColumnFamily("11", datas, status))
    {
        return -2;
    }
    HeaderView header;
    for (auto &item : datas)
    {
        if(item.second.empty())
        {
            continue;
        }
        if(!header.ParseFromByte(item.second.data(), item.second.size()))
        {
            return -3;
        }
        std::cout << Bytes2Hex(item.first) << ":" << std::endl;
        std::cout << header.json.dump(4) << std::endl;
    }
    return 0;
}

int main_data_entry(int argc, char **argv)
{
    rocksdb::Status status;
    RocksDBReadOnly db(db_path, status);
    if (!status.ok())
    {
        return -1;
    }
    std::unordered_map<std::string, std::string> datas;
    if (!db.GetAllDataByColumnFamily("12", datas, status))
    {
        return -2;
    }
    CellDataEntry entry;
    for (auto &item : datas)
    {
        if(item.second.empty())
        {
            continue;
        }
        if(!entry.ParseFromByte(item.second.data(), item.second.size()))
        {
            return -3;
        }
        std::cout << Bytes2Hex(item.first) << ":" << std::endl;
        std::cout << entry.json.dump(4) << std::endl;
    }
    return 0;
}

int main_entry(int argc, char **argv)
{
    rocksdb::Status status;
    RocksDBReadOnly db(db_path, status);
    if (!status.ok())
    {
        return -1;
    }
    std::unordered_map<std::string, std::string> datas;
    if (!db.GetAllDataByColumnFamily("10", datas, status))
    {
        return -2;
    }
    CellEntry entry;
    for (auto &item : datas)
    {
        std::cout << Bytes2Hex(item.first) << ":" << std::endl;
        if(!entry.ParseFromByte(item.second.data(), item.second.size()))
        {
            return -3;
        }
        std::cout << entry.json.dump(4) << std::endl;
    }
    return 0;
}

int main_epoch_ext(int argc, char **argv)
{
    rocksdb::Status status;
    RocksDBReadOnly db(db_path, status);
    if (!status.ok())
    {
        return -1;
    }
    std::unordered_map<std::string, std::string> datas;
    if (!db.GetAllDataByColumnFamily("9", datas, status))
    {
        return -2;
    }
    EpochExt ext;
    for (auto &item : datas)
    {
        std::cout << Bytes2Hex(item.first) << ":"<< std::endl;
        if(!ext.ParseFromByte(item.second.data(), item.second.size()))
        {
            return -2;
        }
        std::cout << ext.json.dump(4) << std::endl;
    }
    return 0;
}


int main(int argc, char **argv)
{
    rocksdb::Status status;
    RocksDBReadOnly db(db_path, status);
    if (!status.ok())
    {
        return -1;
    }
    if (argc < 3)
    {
        printf("usage: %s start end", argv[0]);
        return 0;
    }

    nlohmann::json json;
    nlohmann::json block;
    std::string hash_bytes;
    std::string value;
    Header header;
    UncleBlockVec uncles;
    uint32_t txs_len = 0;
    Transaction transaction;
    ProposalShortIdVec proposals;
    std::ofstream fconf;
    for (uint64_t height = std::stoul(std::string(argv[1])); height < std::stoul(std::string(argv[2])); ++height)
    {
        json.clear();
        std::vector<std::string> tx_infos;
        if (!db.ReadData("0", std::string((char *)&height, sizeof(height)), hash_bytes, status))
        {
            return -2;
        }

        if (!db.ReadData("1", hash_bytes, value, status))
        {
            return -3;
        }
        header.Clear();
        if (!header.ParseFromByteWithHash(value.data(), value.size()))
        {
            return -4;
        }
        block["header"] = header.json;

        if (!db.ReadData("3", hash_bytes, value, status))
        {
            return -5;
        }
        uncles.Clear();
        if (!uncles.ParseFromByte(value.data(), value.size()))
        {
            return -6;
        }
        block["uncles"] = uncles.json;

        if (!db.ReadData("13", std::string((char *)&height, sizeof(height)) + hash_bytes, value, status))
        {
            return -7;
        }
        txs_len = 0;
        memcpy(&txs_len, value.data(), value.size());
        for (uint32_t i = 0; i < txs_len; ++i)
        {
            int index = htobe32(i);
            if (!db.ReadData("2", hash_bytes + std::string((char *)&index, sizeof(index)), value, status))
            {
                return -8;
            }
            transaction.Clear();
            if (!transaction.ParseFromByte(value.data(), value.size()))
            {
                return -9;
            }
            tx_infos.push_back(transaction.json.at("hash"));
            block["transactions"].push_back(transaction.json);
        }

        if (!db.ReadData("7", hash_bytes, value, status))
        {
            return -9;
        }
        proposals.Clear();
        if (!proposals.ParseFromByte(value.data(), value.size()))
        {
            return -10;
        }
        block["proposals"] = proposals.json;

        db.ReadData("15", hash_bytes, value, status);
        if (status.ok())
        {
            block["extension"] = Bytes2Hex(value);
        }
        json["block"] = block;

        int tx_index = 0;
        for(auto &item : tx_infos)
        {
            TransactionInfo info;
            db.ReadData("5", Hex2Bytes(item), value, status);
            if (status.ok() && info.ParseFromByte(value.data(), value.size()))
            {
                json["info"].push_back(info.json);
            }

            CellEntry entry;
            db.ReadData("10", Hex2Bytes(item) + std::string((char *)&tx_index, sizeof(tx_index)), value, status);
            if (status.ok() && entry.ParseFromByte(value.data(), value.size()))
            {
                json["entry"].push_back(entry.json);
            }
            CellDataEntry data_entry;
            db.ReadData("12", Hex2Bytes(item) + std::string((char *)&tx_index, sizeof(tx_index)), value, status);
            if (status.ok() && data_entry.ParseFromByte(value.data(), value.size()))
            {
                json["data_entry"].push_back(data_entry.json);
            }

            ++tx_index;
        }
        db.ReadData("6", hash_bytes, value, status);
        BlockExt block_ext;
        if (status.ok() && block_ext.ParseFromByte(value.data(), value.size()))
        {
            json["block_ext"] = block_ext.json;
        }

        fconf.open(std::to_string(height) + ".txt");
        fconf << json.dump(4);
        fconf.close();
    }
    return 0;
}
