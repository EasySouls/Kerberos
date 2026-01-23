#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <functional>

#include "TypeTraits.hpp"

namespace Kerberos::Core::TypedImpl
{
	template<typename ValueType, auto... Conditions>
	class Constraint
	{
	public:
		static_assert(((std::is_convertible_v<decltype(Conditions), const ValueType&>) && ...), "All conditions must be convertible to the ValueType.");

		static constexpr bool IsValid(const ValueType& value)
		{
			return (std::invoke(Conditions, value) && ...);
		}

		static constexpr void EnsureConstraint(const ValueType& value)
		{
			if (!IsValid(value))
			{
				throw std::invalid_argument("Value does not satisfy the specified constraints.");
			}
		}

		static constexpr bool IsEmpty()
		{
			return sizeof...(Conditions) == 0;
		}
	};

	template<typename ValueType, typename TagType, typename ConstraintType = Constraint<ValueType>>
	class Typed final
	{
	public:

		template<typename U = ValueType, EnableIf<IsDefaultConstructible<U>>* = nullptr>
		constexpr Typed() noexcept(IsNothrowDefaultConstructible<ValueType>);

		template<typename U = ValueType, EnableIf<IsMoveConstructible<U>>* = nullptr>
		constexpr explicit Typed(ValueType&& value) noexcept(IsNothrowMoveConstructible<ValueType>);

		template<typename U = ValueType, EnableIf<IsCopyConstructible<U>>* = nullptr>
		constexpr explicit Typed(const ValueType& value) noexcept(IsNothrowCopyConstructible<ValueType>);

		template<typename... Args, EnableIf<IsConstructible<ValueType, Args...>>* = nullptr>
		constexpr explicit Typed(std::in_place_t, Args&&... args) noexcept(IsNothrowConstructible<ValueType, Args...>);

		template<bool E = ConstraintType::IsEmpty(), EnableIf<!E>* = nullptr>
		static bool IsValid(const ValueType& value);

		template<bool E = ConstraintType::IsEmpty(), EnableIf<!E>* = nullptr>
		constexpr ValueType& Get() & noexcept kbr_lifebound;
		constexpr ValueType&& Get() && noexcept kbr_lifebound;
		constexpr const ValueType& Get() const& noexcept kbr_lifebound;

		template<bool E = ConstraintType::IsEmpty(), EnableIf<!E>* = nullptr>
		constexpr ValueType& operator*() & noexcept kbr_lifebound;
		constexpr ValueType&& operator*() && noexcept kbr_lifebound;
		constexpr const ValueType& operator*() const& noexcept kbr_lifebound;

		template<bool E = ConstraintType::IsEmpty(), EnableIf<!E>* = nullptr>
		constexpr ValueType* operator->() noexcept kbr_lifebound;
		constexpr const ValueType* operator->() const noexcept kbr_lifebound;

	private:
		static_assert(!std::is_reference_v<ValueType>, "Typed<ValueType, TagType> cannot be instantiated with a reference type.");
		static_assert(IsDecayed<ValueType>, "Typed<ValueType, TagType> requires a decayed ValueType.");

		ValueType m_Value{};
	};

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <typename U, EnableIf<IsDefaultConstructible<U>>*>
	constexpr Typed<ValueType, TagType, ConstraintType>::Typed() 
		noexcept(IsNothrowDefaultConstructible<ValueType>) 
	{
		ConstraintType::EnsureConstraint(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <typename U, EnableIf<IsMoveConstructible<U>>*>
	constexpr Typed<ValueType, TagType, ConstraintType>::Typed(ValueType&& value) 
		noexcept(IsNothrowMoveConstructible<ValueType>)
		: m_Value(std::move(value)) // static_cast<ValueType&&>(value))
	{
		ConstraintType::EnsureConstraint(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <typename U, EnableIf<IsCopyConstructible<U>>*>
	constexpr Typed<ValueType, TagType, ConstraintType>::Typed(const ValueType& value) 
		noexcept(IsNothrowCopyConstructible<ValueType>)
		: m_Value(value)
	{
		ConstraintType::EnsureConstraint(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <typename ... Args, EnableIf<IsConstructible<ValueType, Args...>>*>
	constexpr Typed<ValueType, TagType, ConstraintType>::Typed(std::in_place_t, Args&&... args) 
		noexcept(IsNothrowConstructible<ValueType, Args...>) 
		: m_Value(std::forward<Args>(args)...)
	{
		ConstraintType::EnsureConstraint(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <bool E, EnableIf<!E>*>
	bool Typed<ValueType, TagType, ConstraintType>::IsValid(const ValueType& value) 
	{
		return ConstraintType::IsValid(value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <bool E, EnableIf<!E>*>
	constexpr ValueType& Typed<ValueType, TagType, ConstraintType>::Get() & noexcept 
	{
		return m_Value;
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	constexpr ValueType&& Typed<ValueType, TagType, ConstraintType>::Get() && noexcept 
	{
		return std::move(m_Value); // static_cast<ValueType&&>(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	constexpr const ValueType& Typed<ValueType, TagType, ConstraintType>::Get() const & noexcept 
	{
		return m_Value;
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <bool E, EnableIf<!E>*>
	constexpr ValueType& Typed<ValueType, TagType, ConstraintType>::operator*() & noexcept 
	{
		return m_Value;
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	constexpr ValueType&& Typed<ValueType, TagType, ConstraintType>::operator*() && noexcept 
	{
		return std::move(m_Value); // static_cast<ValueType&&>(m_Value);
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	constexpr const ValueType& Typed<ValueType, TagType, ConstraintType>::operator*() const & noexcept 
	{
		return m_Value;
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	template <bool E, EnableIf<!E>*>
	constexpr ValueType* Typed<ValueType, TagType, ConstraintType>::operator->() noexcept 
	{
		return &m_Value;
	}

	template <typename ValueType, typename TagType, typename ConstraintType>
	constexpr const ValueType* Typed<ValueType, TagType, ConstraintType>::operator->() const noexcept 
	{
		return &m_Value;
	}
}

namespace Kerberos::Core {

	template<typename ValueType, typename TagType>
	using Typed = TypedImpl::Typed<ValueType, TagType>;

}