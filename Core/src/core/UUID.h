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
		UUID(uint64_t uuid);
		UUID(const std::string& uuidStr); // Constructor desde cadena
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

		std::string ToString() const; // Convertir UUID a cadena

	private:
		uint64_t m_UUID;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<libCore::UUID>
	{
		std::size_t operator()(const libCore::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}
