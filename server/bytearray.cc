#include "bytearray.h"
#include "log.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

namespace shuai
{

ByteArray::Node::Node()
    :ptr(nullptr)
    ,next(nullptr)
    ,size(0)
{}

ByteArray::Node::Node(size_t s)
    :ptr(new char[s])
    ,next(nullptr)
    ,size(s)
{}

ByteArray::Node::~Node()
{
    if(ptr) delete[] ptr;
    ptr = nullptr;
}

// 每个节点的大小=4k
ByteArray::ByteArray(size_t base_size)
    :m_baseSize(base_size)
    ,m_position(0)
    ,m_capacity(base_size)
    ,m_size(0)
    ,m_endian(SHUAI_BIG_ENDIAN)
    ,m_root(new Node(base_size))
    ,m_cur(m_root)
{}

ByteArray::~ByteArray()
{
    Node* temp = m_root;
    while(temp)
    {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
}

bool ByteArray::isLittleEndian() const
{
    return m_endian == SHUAI_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val)
{
    if(val) m_endian = SHUAI_LITTLE_ENDIAN;
    else m_endian = SHUAI_BIG_ENDIAN;

}

// write 写原始数据
void ByteArray::writeFint8  (int8_t value)
{
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8 (uint8_t value)
{
    write(&value, sizeof(value));
}

void ByteArray::writeFint16 (int16_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value)
{
    if(m_endian != SHUAI_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

// ZigZag 编码的目的是将有符号整数转换为无符号整数，以便对小的负数进行更紧凑的编码。
// 该方法的核心思想是将负数映射到正数，同时保留非负数的编码方式，以减小数据的大小
static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

// 写压缩数据(protobuf)
void ByteArray::writeInt32  (int32_t value)
{
    writeUint32(EncodeZigzag32(value));
}

// uint32_t一般占4个字节；在 可变长编码（variable-length encoding，常用于序列化数据或压缩数据）中，uint32_t 可以占用 最多 5 个字节
// 可变长编码（如 Varint 编码）是一种压缩数字表示法，用来减少存储和传输时的空间开销。具体来说，它将较小的数字用较少的字节表示，而较大的数字则用更多字节表示。
void ByteArray::writeUint32 (uint32_t value)
{
    uint8_t tmp[5]; // tmp[i]中的低七位存储数据，最高位位标志位，表示是否还有更多的数据
    uint8_t i = 0;  // 统计tmp已使用的字节数 
    while(value >= 0x80)   // 保证此时的数据要大于等于10000000=128时才会进入循环
    {
        tmp[i++] = (value & 0x7F) | 0x80;  // 处理value中的最低7位 0x7F:0111 1111  0x80:1000 000   | 0x80就是将temp[i]的最高位置为1
        value >>= 7;   // 将源数据右移七位，处理剩余的数据
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64  (int64_t value)
{
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64 (uint64_t value)
{
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80)
    {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}


void ByteArray::writeFloat  (float value)
{
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble (double value)
{
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFint64(v);
}


// length:int16, data
void ByteArray::writeStringF16(const std::string& value)
{
    writeFuint16(value.size());   // 将字符串的长度写入字节数组
    write(value.c_str(), value.size());  
}

// length:int32, data
void ByteArray::writeStringF32(const std::string& value)
{
    writeFuint32(value.size());
    write(value.c_str(), value.size());     
}

// length:int64, data
void ByteArray::writeStringF64(const std::string& value)
{
    writeFuint64(value.size());
    write(value.c_str(), value.size());     
}

// length:varint, data // 用压缩的长度表示
void ByteArray::writeStringVint(const std::string& value)
{
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

// data
void ByteArray::writeStringWithoutLength(const std::string& value)
{
    write(value.c_str(), value.size());
}

// read
int8_t   ByteArray::readFint8()
{
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t  ByteArray::readFuint8()
{
    int8_t v;
    read(&v, sizeof(v));
    return v;   
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if(m_endian == SHUAI_BYTE_ORDER) \
        return v; \
    return byteswap(v);

int16_t  ByteArray::readFint16()
{
    XX(int16_t);
}

uint16_t ByteArray::readFuint16()
{
    XX(uint16_t);
}

int32_t  ByteArray::readFint32()
{
    XX(int32_t);
}

uint32_t ByteArray::readFuint32()
{
    XX(uint32_t);
}

int64_t  ByteArray::readFint64()
{
    XX(int64_t);
}

uint64_t ByteArray::readFuint64()
{
    XX(uint64_t);
}

#undef XX

int32_t  ByteArray::readInt32()
{
    return DecodeZigzag32(readUint32());  // 写的时候就是先将有符号的转为无符号的再将无符号的进行压缩再写入的 所以读的时候也要按照特定的顺序
}

uint32_t ByteArray::readUint32()
{
    uint32_t result = 0;
    // 每次增加 7。这是因为 Varint 编码的一个特性是每个字节只存储 7 位有效数据，而第 8 位是用来标记是否有更多字节
    for(int i = 0; i < 32; i += 7) 
    {
        uint8_t b = readFint8();
        // 判断 b 的最高位（第 8 位）是否为 0
        if(b < 0x80)
        {
            // 如果最高位为 0，则将 b 直接左移 i 位，并与 result 进行按位或操作
            result |= ((uint32_t)b) << i;
            break;
        }
        else
            result |= ((uint32_t)(b & 0x7f)) << i; // b & 0x7f:将b的最高位置为0
    }
    return result;
}

int64_t  ByteArray::readInt64()
{
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64()
{
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7)
    {
        uint8_t b = readFint8();
        if(b < 0x80)
        {
            result |= ((uint64_t)b) << i;
            break;
        }
        else
            result |= ((uint64_t)(b & 0x7f)) << i;
    }
    return result;    
}


float    ByteArray::readFloat()
{
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double   ByteArray::readDouble()
{
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;   
}


// length: int16, data
std::string ByteArray::readStringF16()
{
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

// length: int32, data
std::string ByteArray::readStringF32()
{
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;    
}

// length: int64, data
std::string ByteArray::readStringF64()
{
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;    
}

// length: varint, data
std::string ByteArray::readStringVint()
{
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;    
}


// 内部操作
void ByteArray::clear()
{
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp)
    {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = NULL;
}


void ByteArray::write(const void* buf, size_t size)
{
    if(size == 0) return;

    addCapacity(size);
    size_t npos = m_position % m_baseSize;  // m_position当前处于所在内存块的哪个位置
    size_t ncap = m_cur->size - npos;       // 当前节点剩余容量
    size_t bpos = 0;                        // buf的位置

    while(size > 0)
    {
        if(ncap >= size)
        {
            memcpy(m_cur->ptr + npos, (char*)buf + bpos, size);
            if(m_cur->size == (npos + size))
                m_cur = m_cur->next;
            m_position += size;
            bpos += size;
            size = 0;
        }
        else{
            memcpy(m_cur->ptr + npos, (char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;   // buf已写入的位置
            size -= ncap;   // 剩余未写入的大小
            m_cur = m_cur->next;  // 未写入的数据将存储在下一个节点
            ncap = m_cur->size;   // 节点的容量为节点的大小
            npos = 0;
        }
    }    

    if(m_position > m_size)
        m_size = m_position;
}

void ByteArray::read(void* buf, size_t size)
{
    if(size > getReadSize())
    {
        throw std::out_of_range("not enough len");
    }
    
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    while(size > 0)
    {
        if(ncap >= size)
        {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if(m_cur->size == (npos + size))
                m_cur = m_cur->next;
            m_position += size;
            bpos += size;
            size = 0;
        }
        else{
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;   
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t position) const
{
    if(size > m_size - position)
    {
        throw std::out_of_range("not enough len");
    }
    
    size_t npos = position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    Node* cur = m_cur;
    while(size > 0)
    {
        if(ncap >= size)
        {
            memcpy((char*)buf + bpos, cur->ptr + npos, size);
            if(cur->size == (npos + size))
                cur = cur->next;
            position += size;
            bpos += size;
            size = 0;
        }
        else{
            memcpy((char*)buf + bpos, cur->ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;   
        }
    }   
}

void ByteArray::setPosition(size_t v)
{
    if(v > m_capacity)
    {
        throw std::out_of_range("set position out of range");
    }
    m_position = v;
    if(m_position > m_size) m_size = m_position;
    m_cur = m_root;
    while(v > m_cur->size)
    {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if(v == m_cur->size)
        m_cur = m_cur->next;
}

bool ByteArray::writeToFile(const std::string& name) const
{
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if(!ofs)
    {
        SHUAI_LOG_ERROR(g_logger) << "writeToFile name = " << name 
                                  << " error, errno = " << errno 
                                  << " errstr = " << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;

    while(read_size > 0)
    {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }

    return true;
}

bool ByteArray::readFromFile(const std::string& name)
{
    std::ifstream ifs(name, std::ios::binary);
    if(!ifs)
    {
        SHUAI_LOG_ERROR(g_logger) << "readFromFile name = " << name   
                                  << " error, errno = " << errno
                                  << " errstr = " << strerror(errno);
        return false;
    }    

    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr){ delete[] ptr; });
    while(!ifs.eof())
    {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size)
{
    if(size == 0) return;
    size_t old_cap = getCapacity();
    if(old_cap >= size) return;

    size = size - old_cap;
    // count表示需要几个内存块
    size_t count = (size / m_baseSize) + (((size % m_baseSize) > old_cap) ? 1 : 0);
    Node* tmp = m_root;
    // 找到最后一个内存块
    while(tmp->next)
        tmp = tmp->next;

    // 添加count个新内存块
    Node* first = NULL;
    for(size_t i = 0; i < count; ++i)
    {
        tmp->next = new Node(m_baseSize);
        if(first == NULL) first = tmp->next;
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }

    if(old_cap == 0)
        m_cur = first;
}

std::string ByteArray::toString() const
{
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) return str;

    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toHexString()
{   
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0; i < str.size(); ++i)
    {
        if(i > 0 && i % 32 == 0) ss << std::endl;
        // std::setw(2) 设置输出宽度为2，而 std::setfill('0') 将填充字符设为 0
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i] << " ";  
    }

    return ss.str();
}

/**
 * @brief 获取可读取的缓存,保存成iovec数组,从position位置开始
 * @param[out] buffers 保存可读取数据的iovec数组
 * @param[in] len 读取数据的长度,如果len > getReadSize() 则 len = getReadSize()
 * @param[in] position 读取数据的位置
 * @return 返回实际数据的长度
 */
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const
{
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) return 0;

    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;

    while(len > 0)
    {
        if(ncap >= len)
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const
{
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) return 0;

    uint64_t size = len;

    size_t npos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node* cur = m_root;
    while(count > 0)
    {
        cur = cur->next;
        --count;
    }

    size_t ncap = cur->size - npos;
    struct iovec iov;
    cur = m_cur;

    while(len > 0)
    {
        if(ncap >= len)
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len)
{
    if(len == 0) return 0;

    addCapacity(len);
    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while(len > 0)
    {
        if(ncap >= len)
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else{
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos=  0;
        }
        buffers.push_back(iov);
    }
    return size;
}

}