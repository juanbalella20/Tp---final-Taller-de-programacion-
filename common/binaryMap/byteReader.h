#ifndef BYTE_READER_H
#define BYTE_READER_H

#include <cstddef>
#include <cstdint>
#include <string>

// Lector secuencial sobre un buffer de bytes en memoria.
// Lee enteros big-endian y valida limites para detectar truncamiento/corrupcion.
class ByteReader {
public:
    ByteReader(const uint8_t* data, size_t size);

    size_t pos() const;
    size_t remaining() const;
    void require(size_t n) const;

    uint16_t read_u16();
    uint32_t read_u32();
    uint8_t read_u8();
    std::string read_string();  // uint16 len + len bytes 

    void skip(size_t n);

private:
    const uint8_t* data_;
    size_t size_;
    size_t pos_ = 0;
};

#endif  // BYTE_READER_H
