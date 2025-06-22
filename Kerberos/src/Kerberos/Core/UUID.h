#pragma once

#include <xhash>

namespace Kerberos
{
	class UUID
	{
	public:
		/**
		* Creates a random 64-bit UUID.
		*/
		UUID();

		/**
		* @brief	Creates a UUID with the given value. 
		* It is the caller's job top make sure that the value is not used.
		*/
		explicit UUID(uint64_t uuid);

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Kerberos::UUID>
	{
		std::size_t operator()(const Kerberos::UUID& uuid) const noexcept
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}