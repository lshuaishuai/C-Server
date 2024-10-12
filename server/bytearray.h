#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>

#include <sys/types.h>
#include <sys/uio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "endian.h"

namespace shuai
{

/**
 * @brief 二进制数组,提供基础类型的序列化,反序列化功能
 */
class ByteArray
{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    // 链表的形式存储
    struct Node
    {
        Node();
        Node(size_t s);
        ~Node();

        char* ptr;       // 内存块地址指针 负责存储数据
        Node* next;
        size_t size;     // 节点大小
    };

    ByteArray(size_t base_size = 4096);  // 每个节点的大小默认=4k
    ~ByteArray();

    // write 写原始数据
    void writeFint8  (int8_t value);
    void writeFuint8 (uint8_t value);
    void writeFint16 (int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32 (int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64 (int64_t value);
    void writeFuint64(uint64_t value);

    // 写压缩数据(protobuf)
    void writeInt32  (int32_t value);
    void writeUint32 (uint32_t value);
    void writeInt64  (int64_t value);
    void writeUint64 (uint64_t value);

    void writeFloat  (float value);
    void writeDouble (double value);

    // length:int16, data
    void writeStringF16(const std::string& value);
    // length:int32, data
    void writeStringF32(const std::string& value);
    // length:int64, data
    void writeStringF64(const std::string& value);
    // length:varint, data
    void writeStringVint(const std::string& value);   // 用压缩的长度表示
    // data
    void writeStringWithoutLength(const std::string& value);   

    // read
    int8_t   readFint8();
    uint8_t  readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int32_t  readFint32();
    uint32_t readFuint32();
    int64_t  readFint64();
    uint64_t readFuint64();

    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();

    float    readFloat();
    double   readDouble();

    // length: int16, data
    std::string readStringF16();
    // length: int32, data
    std::string readStringF32();
    // length: int64, data
    std::string readStringF64();
    // length: varint, data
    std::string readStringVint();

    // 内部操作
    void clear();

    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);
    void read(void* buf, size_t size, size_t position) const;
    
    size_t getPosition() const { return m_position; }
    void setPosition(size_t v);

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const { return m_baseSize; }
    size_t getReadSize() const { return m_size - m_position; }

    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);

    std::string toString() const;
    std::string toHexString();

    // 只获取内容 不修改position
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
    // 增加容量，不修改position
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    size_t getSize() const { return m_size; }

private:
    void addCapacity(size_t size);
    size_t getCapacity() const { return m_capacity - m_position; }  // 得到当前的可放数据的容量

private:
    size_t m_baseSize;   // 大小
    size_t m_position;   // 当前操作的位置
    size_t m_capacity;   // bytearray的总容量
    size_t m_size;       // 当前数据的大小

    int8_t m_endian;     // 字节序，默认为大端

    Node* m_root;        // 第一个内存块指针
    Node* m_cur;         // 当前操作的内存块指针

};
}