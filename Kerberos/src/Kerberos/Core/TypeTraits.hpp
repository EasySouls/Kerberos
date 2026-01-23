#pragma once

#include <type_traits>

#include "Definitions.hpp"

namespace Kerberos {

	template<bool B, class T = void>
	using EnableIf = std::enable_if_t<B, T>;

	template<class T>
	using Decay = std::decay_t<T>;

	template<class T, class... Args>
	constexpr bool IsConstructible = std::is_constructible_v<T, Args...>;

	template<class T, class... Args>
	constexpr bool IsTriviallyConstructible = std::is_trivially_constructible_v<T, Args...>;

	template<class T, class... Args>
	constexpr bool IsNothrowConstructible = std::is_nothrow_constructible_v<T, Args...>;

	template<class T>
	constexpr bool IsDefaultConstructible = std::is_default_constructible_v<T>;

	template<class T>
	constexpr bool IsTriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

	template<class T>
	constexpr bool IsNothrowDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

	template<class T>
	constexpr bool IsCopyConstructible = std::is_copy_constructible_v<T>;

	template<class T>
	constexpr bool IsTriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

	template<class T>
	constexpr bool IsNothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

	template<class T>
	constexpr bool IsMoveConstructible = std::is_move_constructible_v<T>;

	template<class T>
	constexpr bool IsTriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

	template<class T>
	constexpr bool IsNothrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;

	template<class T1, class T2>
	constexpr bool IsSame = std::is_same_v<T1, T2>;

	template<class T>
	constexpr bool IsDecayed = IsSame<Decay<T>, T>;

}