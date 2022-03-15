#ifndef _TYPE_BLOCKCHAIN_H_
#define _TYPE_BLOCKCHAIN_H_

#include <nlohmann/json.hpp>
#include <string>

struct ProtocalBase
{
    nlohmann::json json;
    void Clear();
    virtual bool ParseFromByte(char const *const ptr, size_t size) = 0;
};

struct BytesVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct Byte32Vec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct Script : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct OutPoint : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellInput : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellInputVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};
struct CellOutput : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellOutputVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellDep : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellDepVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};
struct RawTransaction : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct Transaction : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct RawHeader : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct Header : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
    bool ParseFromByteWithHash(char const *const ptr, size_t size);
};

struct ProposalShortIdVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct UncleBlock : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct UncleBlockVec : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};






struct TransactionKey : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct TransactionInfo : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct BlockExt : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct EpochExt : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellEntry : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct CellDataEntry : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};

struct HeaderView : public ProtocalBase
{
    bool ParseFromByte(char const *const ptr, size_t size);
};


#endif
