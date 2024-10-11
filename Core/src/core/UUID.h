#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <iomanip>

namespace libCore {

    class UUID
    {
    public:
        UUID();
        UUID(uint32_t uuid);  // Cambiar a 24 bits
        UUID(const std::string& uuidStr); // Constructor desde cadena
        UUID(const UUID&) = default;

        operator uint32_t() const { return m_UUID; }  // Cambiar a 24 bits

        std::string ToString() const; // Convertir UUID a cadena

    private:
        uint32_t m_UUID;  // Cambiar a 24 bits
    };

}

namespace std {
    template <typename T> struct hash;

    template<>
    struct hash<libCore::UUID>
    {
        std::size_t operator()(const libCore::UUID& uuid) const
        {
            return (uint32_t)uuid;  // Cambiar a 24 bits
        }
    };
}
