#include "UUID.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace libCore {

    static std::random_device s_RandomDevice;
    static std::mt19937 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<uint32_t> s_UniformDistribution(0, 0xFFFFFF);  // Limitar a 24 bits

    UUID::UUID()
        : m_UUID(s_UniformDistribution(s_Engine) & 0xFFFFFF)  // Limitar el UUID a 24 bits
    {
    }


    UUID::UUID(uint32_t uuid)
        : m_UUID(uuid)  // Constructor desde uint32_t
    {
    }

    UUID::UUID(const std::string& uuidStr)
    {
        std::stringstream ss;
        ss << std::hex << uuidStr;
        ss >> m_UUID;
    }

    std::string UUID::ToString() const
    {
        std::stringstream ss;
        ss << std::hex << std::setw(6) << std::setfill('0') << m_UUID;  // Mostrar solo 6 dígitos hexadecimales (24 bits)
        return ss.str();
    }
}
