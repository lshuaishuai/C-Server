#include "server/bytearray.h"
#include "server/server.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test()
{
// 随机生成len个数字，将数字放入vec中，实例化一个byteArray的对象ba，节点大小为base_len 将vec中的数据写入ba 设置初始位置为0 从ba中读出这些数据 
#define XX(type, len, write_fun, read_fun, base_len){ \
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) \
    { \
        vec.push_back(rand()); \
    } \
    shuai::ByteArray::ptr ba(new shuai::ByteArray(base_len)); \
    for(auto& i : vec) \
    { \
        ba->write_fun(i); \
    } \
    ba->setPosition(0); \
    for(size_t i = 0; i < vec.size(); ++i) \
    { \
        type v = ba->read_fun(); \
        SHUAI_ASSERT(v == vec[i]); \
    } \
    SHUAI_ASSERT(ba->getReadSize() == 0); \
    SHUAI_LOG_INFO(g_logger) << #write_fun "/" << #read_fun " (" #type " ) len = " << len \
                             << " base len = " << base_len \
                             << " size = " << ba->getSize(); \
}

    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeInt32, readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t, 100, writeInt64, readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);    
#undef XX

#define SS(type, len, write_fun, read_fun, base_len){ \
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) \
    { \
        vec.push_back(rand()); \
    } \
    shuai::ByteArray::ptr ba(new shuai::ByteArray(base_len)); \
    for(auto& i : vec) \
    { \
        ba->write_fun(i); \
    } \
    ba->setPosition(0); \
    for(size_t i = 0; i < vec.size(); ++i) \
    { \
        type v = ba->read_fun(); \
        SHUAI_ASSERT(v == vec[i]); \
    } \
    SHUAI_ASSERT(ba->getReadSize() == 0); \
    SHUAI_LOG_INFO(g_logger) << #write_fun "/" << #read_fun " (" #type " ) len = " << len \
                             << " base len = " << base_len \
                             << " size = " << ba->getSize(); \
    ba->setPosition(0); \
    SHUAI_ASSERT(ba->writeToFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    shuai::ByteArray::ptr ba2(new shuai::ByteArray(base_len * 2)); \
    SHUAI_ASSERT(ba2->readFromFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    ba2->setPosition(0); \
    SHUAI_ASSERT(ba->toString() == ba2->toString()); \
    SHUAI_ASSERT(ba->getPosition() == 0); \
    SHUAI_ASSERT(ba2->getPosition() == 0); \
}

    SS(int8_t, 100, writeFint8, readFint8, 1);
    SS(uint8_t, 100, writeFuint8, readFuint8, 1);
    SS(int16_t, 100, writeFint16, readFint16, 1);
    SS(uint16_t, 100, writeFuint16, readFuint16, 1);
    SS(int32_t, 100, writeFint32, readFint32, 1);
    SS(uint32_t, 100, writeFuint32, readFuint32, 1);
    SS(int64_t, 100, writeFint64, readFint64, 1);
    SS(uint64_t, 100, writeFuint64, readFuint64, 1);

    SS(int32_t, 100, writeInt32, readInt32, 1);
    SS(uint32_t, 100, writeUint32, readUint32, 1);
    SS(int64_t, 100, writeInt64, readInt64, 1);
    SS(uint64_t, 100, writeUint64, readUint64, 1);    
#undef SS


}

int main(int argc, char** argv)
{
    test();
    return 0;
}