////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016-2026
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   type_traits.h
//  Created:     2016/04/02 by Albert D Yang
//  Description:
// -------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
// -------------------------------------------------------------------------
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
// -------------------------------------------------------------------------
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <tuple>

namespace luabind
{
	template <class _Ty>
	struct default_value
	{
		static _Ty make() noexcept
		{
			return _Ty();
		}
	};

	template <class _Ty>
	struct default_enum
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");

		static _Ty make() noexcept
		{
			return (_Ty)0;
		}
	};

	template <class _Ty>
	struct default_number
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");

		static _Ty make() noexcept
		{
			return (_Ty)0;
		}
	};

	template <class _Ty>
	struct default_object
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static _Ty make() noexcept
		{
			return _Ty();
		}
	};

	template <class _Ty>
	struct default_maker: std::conditional<std::is_enum<_Ty>::value,
		default_enum<_Ty>, typename std::conditional<std::is_arithmetic<_Ty>::value,
		default_number<_Ty>, typename std::conditional<std::is_class<_Ty>::value,
		default_object<_Ty>, default_value<_Ty>>::type>::type>::type
	{
		
	};

	/*struct default: std::conditional < std::is_enum<_Ty>::value,
		default_enum<_Ty>, std::conditional < std::is_arithmetic<_Ty>::value,
		default_number<_Ty>, std::conditional<std::is_class<_Ty>::value,
		default_object<_Ty>, default_value<_Ty >> ::type>::type> ::type*/

	template <>
	struct default_value<void>
	{
		static void make() noexcept
		{

		}
	};

	template <>
	struct default_value<bool>
	{
		static bool make() noexcept
		{
			return false;
		}
	};

	template <>
	struct default_value<const char*>
	{
		static const char* make() noexcept
		{
			return "";
		}
	};

	template <class... _Types>
	struct default_value<std::tuple<_Types...>>;

	template <>
	struct default_value<std::tuple<>>
	{
		static std::tuple<> make() noexcept
		{
			return std::tuple<>();
		}
	};

	template <class _This, class... _Rest>
	struct default_value<std::tuple<_This, _Rest...>>
	{
		static std::tuple<_This, _Rest...> make() noexcept
		{
			return std::tuple_cat(std::make_tuple(default_value<_This>::make()),
				default_value<std::tuple<_Rest...>>::make());
		}
	};


}
