////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
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
	struct enum_traits
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tointeger(L, idx);
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			lua_pushinteger(L, val);
			return 1;
		}

		static _Ty make_default() noexcept
		{
			return (_Ty)0;
		}
	};

	struct bool_traits
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TBOOLEAN;
		}

		static bool get(lua_State* L, int idx) noexcept
		{
			return lua_toboolean(L, idx) ? true : false;
		}

		static int push(lua_State* L, bool val) noexcept
		{
			lua_pushboolean(L, val ? 1 : 0);
			return 1;
		}

		static bool make_default() noexcept
		{
			return false;
		}
	};	

	template <class _Ty>
	struct int_traits
	{
		static_assert(std::is_integral<_Ty>::value, "_Ty is not a integer.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tointeger(L, idx);
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			lua_pushinteger(L, val);
			return 1;
		}

		static _Ty make_default() noexcept
		{
			return 0;
		}
	};

	template <class _Ty>
	struct float_traits
	{
		static_assert(std::is_floating_point<_Ty>::value, "_Ty is not a float.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tonumber(L, idx);
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			lua_pushnumber(L, val);
			return 1;
		}

		static _Ty make_default() noexcept
		{
			return 0;
		}
	};

	template <class _Ty>
	struct number_traits : std::conditional < std::is_same<_Ty, bool>::value,
		bool_traits, typename std::conditional < std::is_integral<_Ty>::value,
		int_traits<_Ty>, float_traits<_Ty >> ::type>::type
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");
	};

	template <class _Ty>
	struct object_traits
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 0;

		static bool test(lua_State* L, int idx) noexcept
		{
			return false;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return make_default();
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			return 0;
		}

		static _Ty make_default() noexcept
		{
			return _Ty();
		}
	};

	template <class _Ty>
	struct object_ptr_traits
	{
		static_assert(std::is_pointer<_Ty>::value
			&& std::is_class<typename std::remove_pointer<_Ty>::type>::value,
			"_Ty is not a class pointer.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 0;

		static bool test(lua_State* L, int idx) noexcept
		{
			return false;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return make_default();
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			return 0;
		}

		static _Ty make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct value_ptr_traits
	{
		static_assert(std::is_pointer<_Ty>::value, "_Ty is not a pointer.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 0;

		static bool test(lua_State* L, int idx) noexcept
		{
			return false;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return make_default();
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			return 0;
		}

		static _Ty make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct pointer_traits : std::conditional <
		std::is_class<typename std::remove_pointer<_Ty>::type>::value,
		object_ptr_traits<_Ty>, value_ptr_traits < _Ty >> ::type
	{
		static_assert(std::is_pointer<_Ty>::value, "_Ty is not a pointer.");

	};

	template <class _Ty>
	struct value_traits
	{
		static constexpr bool can_get = false;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 0;
	};

	template <class _Ty>
	struct type_traits : std::conditional < std::is_enum<_Ty>::value,
		enum_traits<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		number_traits<_Ty>, typename std::conditional<std::is_class<_Ty>::value,
		object_traits<_Ty>, typename std::conditional<std::is_pointer<_Ty>::value,
		pointer_traits<_Ty>, value_traits<_Ty >> ::type> ::type>::type>::type
	{

	};

	template <>
	struct value_traits<void>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 0;

		static bool test(lua_State* L, int idx) noexcept
		{
			return true;
		}

		static void get(lua_State* L, int idx) noexcept
		{
			
		}

		static void make_default() noexcept
		{

		}
	};

	template <>
	struct value_ptr_traits<char*>
	{
		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return false;
		}

		static char* get(lua_State* L, int idx) noexcept
		{
			return make_default();
		}

		static int push(lua_State* L, char* val) noexcept
		{
			lua_pushstring(L, val);
			return 1;
		}

		static char* make_default() noexcept
		{
			return nullptr;
		}
	};

	template <>
	struct value_ptr_traits<const char*>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return (lua_type(L, idx) == LUA_TSTRING);
		}

		static const char* get(lua_State* L, int idx) noexcept
		{
			return lua_tostring(L, idx);
		}

		static int push(lua_State* L, const char* val) noexcept
		{
			lua_pushstring(L, val);
			return 1;
		}

		static const char* make_default() noexcept
		{
			return "";
		}
	};

	template <>
	struct value_ptr_traits<void*>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TLIGHTUSERDATA;
		}

		static void* get(lua_State* L, int idx) noexcept
		{
			return lua_touserdata(L, idx);
		}

		static int push(lua_State* L, void* val) noexcept
		{
			lua_pushlightuserdata(L, val);
			return 1;
		}

		static void* make_default() noexcept
		{
			return nullptr;
		}
	};

	template <>
	struct value_ptr_traits<lua_CFunction>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_iscfunction(L, idx) ? true : false;
		}

		static lua_CFunction get(lua_State* L, int idx) noexcept
		{
			return lua_tocfunction(L, idx);
		}

		static int push(lua_State* L, lua_CFunction val) noexcept
		{
			lua_pushcfunction(L, val);
			return 1;
		}

		static lua_CFunction make_default() noexcept
		{
			return nullptr;
		}
	};

	template <>
	struct value_ptr_traits<lua_State*>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TTHREAD;
		}

		static lua_State* get(lua_State* L, int idx) noexcept
		{
			return lua_tothread(L, idx);
		}

		static int push(lua_State* L, lua_CFunction val) noexcept
		{
			return 0;
		}

		static lua_State* make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class... _Types>
	struct can_get_pak;

	template <class... _Types>
	struct can_push_pak;

	template <>
	struct can_get_pak<> : std::true_type
	{

	};

	template <>
	struct can_push_pak<> : std::true_type
	{

	};

	template <class _This, class... _Rest>
	struct can_get_pak<_This, _Rest...>
		: std::conditional<type_traits<_This>::can_get && can_get_pak<_Rest...>::value,
		std::true_type, std::false_type>::type
	{

	};

	template <class _This, class... _Rest>
	struct can_push_pak<_This, _Rest...>
		: std::conditional<type_traits<_This>::can_push && can_push_pak<_Rest...>::value,
		std::true_type, std::false_type>::type
	{

	};

	template <int idx, int size, class... _Rest>
	struct tuple_pusher
	{
		static int push(lua_State *L, std::tuple<_Rest...> tuple) noexcept
		{
			auto val = std::get<idx>(tuple);
			int i = type_traits<decltype(val)>::push(L, val);
			if (i >= 0)
			{
				int j = tuple_pusher<idx + 1, size, _Rest...>::push(L, tuple);
				if (j >= 0)
				{
					return i + j;
				}
			}
			return -1;
		}
	};

	template <size_t size, class... _Rest>
	struct tuple_pusher<size, size, _Rest...>
	{
		static int push(lua_State *L, std::tuple<_Rest...> tuple) noexcept
		{
			return 0;
		}
	};

	template <class... _Types>
	struct object_traits<std::tuple<_Types...>>;

	template <>
	struct object_traits<std::tuple<>>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 0;

		static bool test(lua_State *L, int idx) noexcept
		{
			return true;
		}

		static std::tuple<> get(lua_State *L, int idx) noexcept
		{
			return std::tuple<>();
		}

		static int push(lua_State *L, std::tuple<>) noexcept
		{
			return 0;
		}

		static std::tuple<> make_default() noexcept
		{
			return std::tuple<>();
		}
	};

	template <class _This, class... _Rest>
	struct object_traits<std::tuple<_This, _Rest...>>
	{
		static constexpr bool can_get = can_get_pak<_This, _Rest...>::value;

		static constexpr bool can_push = can_push_pak<_This, _Rest...>::value;

		static constexpr int stack_count = type_traits<_This>::stack_count + type_traits<std::tuple<_Rest...>>::stack_count;

		static bool test(lua_State *L, int idx) noexcept
		{
			if (type_traits<_This>::test(L, idx))
			{
				return type_traits<std::tuple<_Rest...>>::test(L, idx + type_traits<_This>::stack_count);
			}
			return false;
		}

		static std::tuple<_This, _Rest...> get(lua_State *L, int idx) noexcept
		{
			return std::tuple_cat(std::make_tuple(type_traits<_This>::get(L, idx)),
				type_traits<std::tuple<_Rest...>>::get(L, idx + type_traits<_This>::stack_count));
		}

		static int push(lua_State *L, std::tuple<_This, _Rest...> val) noexcept
		{
			return tuple_pusher<0, sizeof...(_Rest) + 1, _This, _Rest...>::push(L, val);
		}

		static std::tuple<_This, _Rest...> make_default() noexcept
		{
			return std::tuple_cat(std::make_tuple(type_traits<_This>::make_default()),
				type_traits<std::tuple<_Rest...>>::make_default());
		}
	};

	template <class... _Types>
	struct params_pusher;

	template<>
	struct params_pusher<>
	{
		static int push(lua_State *L) noexcept
		{
			return 0;
		}
	};

	template <class _This, class... _Rest>
	struct params_pusher<_This, _Rest...>
	{
		static int push(lua_State *L, _This val, _Rest... pak) noexcept
		{
			int i = type_traits<_This>::push(L, val);
			if (i >= 0)
			{
				int j = params_pusher<_Rest...>::push(L, pak...);
				if (j >= 0)
				{
					return i + j;
				}
			}
			return -1;
		}
	};
}
