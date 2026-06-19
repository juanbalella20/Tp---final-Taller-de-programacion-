#include "byteReader.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

ByteReader::ByteReader(const uint8_t* data, size_t size) : data_(data), size_(size) {}

size_t ByteReader::pos() const { return pos_; }

void ByteReader::require(size_t n) const {
    if (pos_ + n > size_) {
        throw std::runtime_error("BinaryMapLoader: archivo truncado/corrupto");
    }
}

uint16_t ByteReader::read_u16() {
    require(2);
    uint16_t network_value = 0;
    std::memcpy(&network_value, data_ + pos_, sizeof(network_value));
    pos_ += 2;
    return ntohs(network_value);
}

uint32_t ByteReader::read_u32() {
    require(4);
    uint32_t network_value = 0;
    std::memcpy(&network_value, data_ + pos_, sizeof(network_value));
    pos_ += 4;
    return ntohl(network_value);
}

uint8_t ByteReader::read_u8() {
    require(1);
    return data_[pos_++];
}

std::string ByteReader::read_string() {
    uint16_t len = read_u16();
    require(len);
    std::string s(reinterpret_cast<const char*>(data_ + pos_), len);
    pos_ += len;
    return s;
}

void ByteReader::skip(size_t n) {
    require(n);
    pos_ += n;
}
