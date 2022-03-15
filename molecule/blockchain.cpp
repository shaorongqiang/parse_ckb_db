#include "blockchain.h"
#include "db/rocksdb_read_only.h"
#include "log/logging.h"
#include "utils/crypto_utils.h"
#include <algorithm>
#include <iostream>
#include <string>

#include "generated/blockchain.h"
#include "generated/extensions.h"

void ProtocalBase::Clear()
{
    json.clear();
}

bool BytesVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_BytesVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_BytesVec_length(&buf);
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_BytesVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        json.push_back(Bytes2Hex(std::string((char *)mol.seg.ptr, mol.seg.size)));
    }
    return true;
}

bool Byte32Vec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_Byte32Vec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_Byte32Vec_length(&buf);
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_Byte32Vec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        json.push_back(Bytes2Hex(std::string((char *)mol.seg.ptr, mol.seg.size)));
    }
    return true;
}

bool Script::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_Script_verify(&buf, 0))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_num_t mol_num = MolReader_Script_actual_field_count(&buf);
    bool flag = MolReader_Script_has_extra_fields(&buf);

    mol_seg_t mol = MolReader_Script_get_code_hash(&buf);
    std::string bytes((char *)mol.ptr, mol.size);
    json["code_hash"] = std::move(Bytes2Hex(bytes));

    uint8_t hash_type = 0;
    mol = MolReader_Script_get_hash_type(&buf);
    memcpy(&hash_type, mol.ptr, mol.size);
    json["hash_type"] = hash_type;

    mol = MolReader_Script_get_args(&buf);
    json["args"] = std::move(Bytes2Hex(bytes));

    return true;
}

bool OutPoint::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;
    if (MOL_OK != MolReader_OutPoint_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_OutPoint_get_tx_hash(&buf);
    std::string bytes((char *)mol.ptr, mol.size);
    json["tx_hash"] = std::move(Bytes2Hex(bytes));

    mol = MolReader_OutPoint_get_index(&buf);
    uint32_t index = 0;
    memcpy(&index, mol.ptr, mol.size);
    json["index"] = index;

    return true;
}

bool CellInput::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellInput_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_CellInput_get_since(&buf);
    uint64_t since;
    memcpy(&since, mol.ptr, mol.size);
    json["since"] = since;
    OutPoint previous_output;
    mol = MolReader_CellInput_get_previous_output(&buf);
    if (mol.size > 0)
    {
        if (!previous_output.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["previous_output"] = previous_output.json;
    }
    return true;
}

bool CellInputVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellInputVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_CellInputVec_length(&buf);
    CellInput input;
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_CellInputVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        input.Clear();
        if (!input.ParseFromByte((char *)mol.seg.ptr, mol.seg.size))
        {
            return false;
        }
        json.push_back(input.json);
    }
    return true;
}

bool CellOutput::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellOutput_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    mol_seg_t mol = MolReader_CellOutput_get_capacity(&buf);
    uint64_t capacity;
    memcpy(&capacity, mol.ptr, mol.size);
    json["capacity"] = capacity;

    mol = MolReader_CellOutput_get_lock(&buf);
    if (mol.size > 0)

    {
        Script lock;
        if (!lock.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["lock"] = lock.json;
    }

    mol = MolReader_CellOutput_get_type_(&buf);
    if (mol.size > 0)
    {
        Script type_;
        if (!type_.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["type_"] = type_.json;
    }
    return true;
}

bool CellOutputVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellOutputVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_CellOutputVec_length(&buf);
    CellOutput output;
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_CellOutputVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        output.Clear();
        if (!output.ParseFromByte((char *)mol.seg.ptr, mol.seg.size))
        {
            return false;
        }
        json.push_back(output.json);
    }
    return true;
}

bool CellDep::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellDep_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    mol_seg_t mol = MolReader_CellDep_get_out_point(&buf);
    OutPoint out_point;
    if (out_point.ParseFromByte((char *)mol.ptr, mol.size))
    {
        json["out_point"] = out_point.json;
    }

    mol = MolReader_CellDep_get_dep_type(&buf);
    uint8_t dep_type = 0;
    memcpy(&dep_type, mol.ptr, mol.size);
    json["dep_type"] = dep_type;

    return true;
}

bool CellDepVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellDepVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_CellDepVec_length(&buf);
    CellDep cell_dep;
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_CellDepVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        cell_dep.Clear();
        if (!cell_dep.ParseFromByte((char *)mol.seg.ptr, mol.seg.size))
        {
            return false;
        }
        json.push_back(cell_dep.json);
    }
    return true;
}

bool RawTransaction::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_RawTransaction_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t version = 0;
    mol_seg_t mol = MolReader_RawTransaction_get_version(&buf);
    memcpy(&version, mol.ptr, mol.size);
    json["version"] = version;

    {
        mol = MolReader_RawTransaction_get_cell_deps(&buf);
        CellDepVec cell_deps;
        if (!cell_deps.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["cell_deps"] = cell_deps.json;
    }
    {
        mol = MolReader_RawTransaction_get_header_deps(&buf);
        Byte32Vec header_deps;
        if (!header_deps.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["header_deps"] = header_deps.json;
    }

    {
        mol = MolReader_RawTransaction_get_inputs(&buf);
        CellInputVec inputs;
        if (!inputs.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["inputs"] = inputs.json;
    }
    {
        mol = MolReader_RawTransaction_get_outputs(&buf);
        CellOutputVec outputs;
        if (!outputs.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["outputs"] = outputs.json;
    }
    {
        mol = MolReader_RawTransaction_get_outputs_data(&buf);
        BytesVec outputs_data;
        if (!outputs_data.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["outputs_data"] = outputs_data.json;
    }
    return true;
}
bool Transaction::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    uint32_t num[5] = {0};
    memcpy(num, ptr, sizeof(num));

    json["hash"] = std::move(Bytes2Hex(std::string(ptr + num[1], num[2] - num[1])));
    json["witnesses"] = std::move(Bytes2Hex(std::string(ptr + num[2], num[3] - num[2])));

    mol_seg_t buf;
    buf.ptr = (uint8_t *)(ptr + num[3]);
    buf.size = size - num[3];
    // TODO has_extra_fields
    if (MOL_OK != MolReader_Transaction_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_Transaction_get_raw(&buf);
    RawTransaction raw;
    if (!raw.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["raw"] = raw.json;
    return true;
}

bool RawHeader::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_RawHeader_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_RawHeader_get_version(&buf);
    uint32_t version;
    memcpy(&version, mol.ptr, mol.size);
    json["version"] = version;

    mol = MolReader_RawHeader_get_compact_target(&buf);
    uint32_t compact_target;
    memcpy(&compact_target, mol.ptr, mol.size);
    json["compact_target"] = compact_target;

    mol = MolReader_RawHeader_get_timestamp(&buf);
    uint64_t timestamp;
    memcpy(&timestamp, mol.ptr, mol.size);
    json["timestamp"] = timestamp;

    mol = MolReader_RawHeader_get_number(&buf);
    uint64_t number;
    memcpy(&number, mol.ptr, mol.size);
    json["number"] = number;

    mol = MolReader_RawHeader_get_epoch(&buf);
    uint64_t epoch;
    memcpy(&epoch, mol.ptr, mol.size);
    json["epoch"] = epoch;

    mol = MolReader_RawHeader_get_parent_hash(&buf);
    json["parent_hash"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    mol = MolReader_RawHeader_get_transactions_root(&buf);
    json["transactions_root"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    mol = MolReader_RawHeader_get_proposals_hash(&buf);
    json["proposals_hash"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    mol = MolReader_RawHeader_get_extra_hash(&buf);
    json["extra_hash"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    mol = MolReader_RawHeader_get_dao(&buf);
    json["dao"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    return true;
}

bool Header::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }

    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_Header_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    {
        mol_seg_t mol = MolReader_Header_get_raw(&buf);
        RawHeader raw;
        if (!raw.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["raw"] = raw.json;
    }

    {
        __uint128_t nonce;
        mol_seg_t mol = MolReader_Header_get_nonce(&buf);
        memcpy(&nonce, mol.ptr, mol.size);
        std::string str;
        while (nonce > 0)
        {
            str.push_back(nonce % 10 + '0');
            nonce /= 10;
        }
        std::reverse(str.begin(), str.end());
        json["nonce"] = str;
    }

    return true;
}
bool Header::ParseFromByteWithHash(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    size_t hash_len = 32;
    json["hash"] = Bytes2Hex(std::string(ptr, hash_len));

    mol_seg_t buf;
    buf.ptr = (uint8_t *)(ptr + hash_len);
    buf.size = size - hash_len;

    if (MOL_OK != MolReader_Header_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    {
        mol_seg_t mol = MolReader_Header_get_raw(&buf);
        RawHeader raw;
        if (!raw.ParseFromByte((char *)mol.ptr, mol.size))
        {
            return false;
        }
        json["raw"] = raw.json;
    }

    {
        __uint128_t nonce;
        mol_seg_t mol = MolReader_Header_get_nonce(&buf);
        memcpy(&nonce, mol.ptr, mol.size);
        std::string str;
        while (nonce > 0)
        {
            str.push_back(nonce % 10 + '0');
            nonce /= 10;
        }
        std::reverse(str.begin(), str.end());
        json["nonce"] = str;
    }

    return true;
}

bool ProposalShortIdVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_ProposalShortIdVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_ProposalShortIdVec_length(&buf);
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_ProposalShortIdVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        json.push_back(Bytes2Hex(std::string((char *)mol.seg.ptr, mol.seg.size)));
    }
    return true;
}

bool UncleBlock::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_UncleBlock_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_UncleBlock_get_header(&buf);
    Header header;
    if (!header.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["header"] = header.json;

    mol = MolReader_UncleBlock_get_proposals(&buf);
    ProposalShortIdVec proposals;
    if (!proposals.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["proposals"] = proposals.json;
    return true;
}

bool UncleBlockVec::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    uint32_t num[4] = {0};
    memcpy(num, ptr, sizeof(num));
    json["hash"] = std::move(Bytes2Hex(std::string(ptr + num[1], num[2] - num[1])));

    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr + num[2];
    buf.size = size - num[2];

    if (MOL_OK != MolReader_UncleBlockVec_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    uint32_t len = MolReader_UncleBlockVec_length(&buf);
    UncleBlock uncle;
    mol_seg_res_t mol;
    for (int i = 0; i < len; i++)
    {
        mol = MolReader_UncleBlockVec_get(&buf, i);
        if (MOL_OK != mol.errno)
        {
            return false;
        }
        uncle.Clear();
        if (!uncle.ParseFromByte((char *)mol.seg.ptr, mol.seg.size))
        {
            return false;
        }
        json["data"].push_back(uncle.json);
    }
    return true;
}

bool TransactionKey::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_TransactionKey_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_TransactionKey_get_block_hash(&buf);
    std::string bytes((char *)mol.ptr, mol.size);
    json["block_hash"] = std::move(Bytes2Hex(bytes));

    mol = MolReader_OutPoint_get_index(&buf);
    uint32_t index = 0;
    memcpy(&index, mol.ptr, mol.size);
    index = htobe32(index);
    json["index"] = index;

    return true;
}

bool TransactionInfo::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_TransactionInfo_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    mol_seg_t mol = MolReader_TransactionInfo_get_block_number(&buf);
    uint64_t block_number = 0;
    memcpy(&block_number, mol.ptr, mol.size);
    json["block_number"] = block_number;

    mol = MolReader_TransactionInfo_get_block_epoch(&buf);
    uint64_t block_epoch = 0;
    memcpy(&block_number, mol.ptr, mol.size);
    json["block_epoch"] = block_epoch;

    mol = MolReader_TransactionInfo_get_key(&buf);
    TransactionKey key;
    if (!key.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["key"] = key.json;
    return true;
}

bool BlockExt::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_BlockExt_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }

    mol_seg_t mol = MolReader_BlockExt_get_total_difficulty(&buf);
    json["total_difficulty"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));

    mol = MolReader_BlockExt_get_total_uncles_count(&buf);
    uint64_t total_uncles_count = 0;
    memcpy(&total_uncles_count, mol.ptr, mol.size);
    json["total_uncles_count"] = total_uncles_count;

    mol = MolReader_BlockExt_get_received_at(&buf);
    uint64_t received_at = 0;
    memcpy(&received_at, mol.ptr, mol.size);
    json["received_at"] = received_at;

    mol = MolReader_BlockExt_get_txs_fees(&buf);

    mol_seg_res_t tmp_mol;
    int len = MolReader_Uint64Vec_length(&mol);
    for (int i = 0; i < len; ++i)
    {
        tmp_mol = MolReader_Uint64Vec_get(&mol, i);
        uint64_t val = 0;
        memcpy(&val, tmp_mol.seg.ptr, tmp_mol.seg.size);
        json["txs_fees"].push_back(val);
    }

    mol = MolReader_BlockExt_get_verified(&buf);
    if (mol.size > 0)
    {
        json["verified"] = Bytes2Hex(std::string((char *)mol.ptr, mol.size));
    }

    return true;
}

bool EpochExt::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_EpochExt_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_EpochExt_get_previous_epoch_hash_rate(&buf);
    json["previous_epoch_hash_rate"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));

    mol = MolReader_EpochExt_get_last_block_hash_in_previous_epoch(&buf);
    json["last_block_hash_in_previous_epoch"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));

    mol = MolReader_EpochExt_get_compact_target(&buf);
    uint32_t compact_target = 0;
    memcpy(&compact_target, mol.ptr, mol.size);
    json["compact_target"] = compact_target;

    mol = MolReader_EpochExt_get_number(&buf);
    uint64_t number = 0;
    memcpy(&number, mol.ptr, mol.size);
    json["number"] = number;

    mol = MolReader_EpochExt_get_base_block_reward(&buf);
    uint64_t base_block_reward = 0;
    memcpy(&base_block_reward, mol.ptr, mol.size);
    json["base_block_reward"] = base_block_reward;

    mol = MolReader_EpochExt_get_remainder_reward(&buf);
    uint64_t remainder_reward = 0;
    memcpy(&remainder_reward, mol.ptr, mol.size);
    json["remainder_reward"] = remainder_reward;

    mol = MolReader_EpochExt_get_start_number(&buf);
    uint64_t start_number = 0;
    memcpy(&start_number, mol.ptr, mol.size);
    json["start_number"] = start_number;

    mol = MolReader_EpochExt_get_length(&buf);
    uint64_t length = 0;
    memcpy(&length, mol.ptr, mol.size);
    json["length"] = length;

    return true;
}

/*able CellEntry {
    output:                CellOutput,
    block_hash:            Byte32,
    block_number:          Uint64,
    block_epoch:           Uint64,
    index:                 Uint32,
    data_size:             Uint64,
}*/

bool CellEntry::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellEntry_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_CellEntry_get_output(&buf);
    CellOutput output;
    if(!output.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["output"] = output.json;

    mol = MolReader_CellEntry_get_block_hash(&buf);
    json["block_hash"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));

    mol = MolReader_CellEntry_get_block_number(&buf);
    uint64_t number = 0;
    memcpy(&number, mol.ptr, mol.size);
    json["number"] = number;

    mol = MolReader_CellEntry_get_block_epoch(&buf);
    uint64_t block_epoch = 0;
    memcpy(&block_epoch, mol.ptr, mol.size);
    json["block_epoch"] = block_epoch;

    mol = MolReader_CellEntry_get_index(&buf);
    uint64_t index = 0;
    memcpy(&index, mol.ptr, mol.size);
    json["index"] = index;

    mol = MolReader_CellEntry_get_data_size(&buf);
    uint64_t data_size = 0;
    memcpy(&data_size, mol.ptr, mol.size);
    json["data_size"] = data_size;

    return true;
}
bool CellDataEntry::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_CellDataEntry_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_CellDataEntry_get_output_data(&buf);
    json["output_data"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));

    mol = MolReader_CellDataEntry_get_output_data_hash(&buf);
    json["output_data_hash"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));
    return true;
}

bool HeaderView::ParseFromByte(char const *const ptr, size_t size)
{
    if (size <= 0)
    {
        return false;
    }
    mol_seg_t buf;
    buf.ptr = (uint8_t *)ptr;
    buf.size = size;

    if (MOL_OK != MolReader_HeaderView_verify(&buf, 1))
    {
        ERRORLOG("verify error");
        Clear();
        return false;
    }
    mol_seg_t mol = MolReader_HeaderView_get_hash(&buf);
    json["hash"] = std::move(Bytes2Hex(std::string((char *)mol.ptr, mol.size)));

    Header header;
    mol = MolReader_HeaderView_get_data(&buf);
    if(!header.ParseFromByte((char *)mol.ptr, mol.size))
    {
        return false;
    }
    json["data"] = header.json;

    return true;
}
