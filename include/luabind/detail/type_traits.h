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

	/*template <class _Ty>
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
	struct default_value
	{
		static _Ty make() noexcept
		{
			return _Ty();
		}
	};

	template <class _Ty>
	struct default_maker : std::conditional < std::is_enum<_Ty>::value,
		default_enum<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		default_number<_Ty>, typename std::conditional<std::is_class<_Ty>::value,
		default_object<_Ty>, default_value<_Ty >> ::type>::type>::type
	{

	};

	template <>
	struct default_value<void>
	{
		static void make() noexcept
		{

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

	template <class _Ty>
	struct default_value<_Ty*>
	{
		static _Ty* make() noexcept
		{
			return nullptr;
		}
	};

	template <class... _Types>
	struct default_object<std::tuple<_Types...>>;

	template <>
	struct default_object<std::tuple<>>
	{
		static std::tuple<> make() noexcept
		{
			return std::tuple<>();
		}
	};

	template <class _This, class... _Rest>
	struct default_object<std::tuple<_This, _Rest...>>
	{
		static std::tuple<_This, _Rest...> make() noexcept
		{
			return std::tuple_cat(std::make_tuple(default_maker<_This>::make()),
				default_maker<std::tuple<_Rest...>>::make());
		}
	};

	template <class _Ty>
	struct can_get_enum : std::true_type
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");
	};

	template <class _Ty>
	struct can_push_enum : std::true_type
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");
		typedef std::true_type type1;
	};

	template <class _Ty>
	struct can_get_number : std::true_type
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");
	};

	template <class _Ty>
	struct can_push_number : std::true_type
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");
	};

	template <class _Ty>
	struct can_get_value : std::false_type
	{
		
	};

	template <class _Ty>
	struct can_push_value : std::false_type
	{
		
	};

	template <class _Ty>
	struct can_get : std::conditional < std::is_enum<_Ty>::value,
		can_get_enum<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		can_get_number<_Ty>, can_get_value<_Ty >> ::type> ::type
	{

	};

	template <class _Ty>
	struct can_push : std::conditional < std::is_enum<_Ty>::value,
		can_push_enum<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		can_push_number<_Ty>, can_push_value<_Ty >> ::type> ::type
	{

	};

	template <>
	struct can_get_value<void> : std::true_type
	{

	};

	template <>
	struct can_get_value<bool> : std::true_type
	{

	};

	template <>
	struct can_push_value<bool> : std::true_type
	{

	};

	template <>
	struct can_get_value<void*> : std::true_type
	{

	};

	template <>
	struct can_push_value<void*> : std::true_type
	{

	};

	template <>
	struct can_get_value<lua_CFunction> : std::true_type
	{

	};

	template <>
	struct can_push_value<lua_CFunction> : std::true_type
	{

	};

	template <>
	struct can_get_value<lua_State*> : std::true_type
	{

	};

	template <>
	struct can_get_value<const char*> : std::true_type
	{

	};

	template <>
	struct can_push_value<const char*> : std::true_type
	{

	};

	template <>
	struct can_push_value<char*> : std::true_type
	{

	};

	template <class... _Types>
	struct can_get_value<std::tuple<_Types...>>;

	template <class... _Types>
	struct can_push_value<std::tuple<_Types...>>;

	template <>
	struct can_get_value<std::tuple<>> : std::true_type
	{

	};

	template <>
	struct can_push_value<std::tuple<>> : std::true_type
	{

	};

	template <class _This, class... _Rest>
	struct can_get_value<std::tuple<_This, _Rest...>>
		: std::conditional<can_get<_This>::value && can_get<std::tuple<_Rest...>>::value,
		std::true_type, std::false_type>::type
	{

	};

	template <class _This, class... _Rest>
	struct can_push_value<std::tuple<_This, _Rest...>>
		: std::conditional<can_push<_This>::value && can_push<std::tuple<_Rest...>>::value,
		std::true_type, std::false_type>::type
	{

	};

	template <class _Ty>
	struct value_getter
	{

	};

	template <class _Ty>
	struct value_pusher
	{

	};

	template <class _Ty>
	struct enum_getter
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tointeger(L, idx);
		}
	};

	template <class _Ty>
	struct enum_pusher
	{
		static_assert(std::is_enum<_Ty>::value, "_Ty is not a enum.");

		static int push(lua_State* L, _Ty val) noexcept
		{			
			lua_pushinteger(L, val);
			return 1;
		}
	};

	struct bool_getter
	{
		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TBOOLEAN;
		}

		static bool get(lua_State* L, int idx) noexcept
		{
			return lua_toboolean(L, idx) ? true : false;
		}
	};

	struct bool_pusher
	{
		static int push(lua_State* L, bool val) noexcept
		{
			lua_pushboolean(L, val ? 1 : 0);
			return 1;
		}
	};

	template <class _Ty>
	struct integer_getter
	{
		static_assert(std::is_integral<_Ty>::value, "_Ty is not a integer.");

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tointeger(L, idx);
		}
	};

	template <class _Ty>
	struct integer_pusher
	{
		static_assert(std::is_integral<_Ty>::value, "_Ty is not a integer.");

		static int push(lua_State* L, _Ty val) noexcept
		{
			lua_pushinteger(L, val);
			return 1;
		}
	};

	template <class _Ty>
	struct float_getter
	{
		static_assert(std::is_floating_point<_Ty>::value, "_Ty is not a float.");

		static bool test(lua_State* L, int idx) noexcept
		{
			return lua_type(L, idx) == LUA_TNUMBER;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return (_Ty)lua_tonumber(L, idx);
		}
	};

	template <class _Ty>
	struct float_pusher
	{
		static_assert(std::is_floating_point<_Ty>::value, "_Ty is not a float.");

		static int push(lua_State* L, _Ty val) noexcept
		{
			lua_pushnumber(L, val);
			return 1;
		}
	};

	template <class _Ty>
	struct number_getter : std::conditional < std::is_same<_Ty, bool>::value,
		bool_getter, typename std::conditional < std::is_integral<_Ty>::value,
		integer_getter<_Ty>, typename std::conditional <std::is_floating_point<_Ty>::value,
		float_getter<_Ty>, value_getter<_Ty >> ::type> ::type> ::type
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");
		static constexpr int stack_count = 1;
	};

	template <class _Ty>
	struct number_pusher : std::conditional < std::is_same<_Ty, bool>::value,
		bool_pusher, typename std::conditional < std::is_integral<_Ty>::value,
		integer_pusher<_Ty>, typename std::conditional <std::is_floating_point<_Ty>::value,
		float_pusher<_Ty>, value_pusher<_Ty >> ::type> ::type> ::type
	{
		static_assert(std::is_arithmetic<_Ty>::value, "_Ty is not a number.");
	};

	template <class _Ty>
	struct param_getter : std::conditional < std::is_enum<_Ty>::value,
		enum_getter<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		number_getter<_Ty>, value_getter<_Ty >> ::type> ::type
	{

	};

	template <class _Ty>
	struct param_pusher : std::conditional < std::is_enum<_Ty>::value,
		enum_pusher<_Ty>, typename std::conditional < std::is_arithmetic<_Ty>::value,
		number_pusher<_Ty>, value_pusher<_Ty >> ::type> ::type
	{

	};

	template <>
	struct value_getter<void>
	{
		static constexpr int stack_count = 0;

		static bool test(lua_State *L, int idx) noexcept
		{
			return true;
		}

		static void get(lua_State *L, int idx) noexcept
		{
			
		}
	};

	template <>
	struct value_getter<void*>
	{
		static constexpr int stack_count = 1;

		static bool test(lua_State *L, int idx) noexcept
		{
			return (lua_type(L, idx) == LUA_TLIGHTUSERDATA);
		}

		static void* get(lua_State *L, int idx) noexcept
		{
			return lua_touserdata(L, idx);
		}
	};

	template <>
	struct value_pusher<void*>
	{
		static int push(lua_State *L, void* val) noexcept
		{
			if (val)
			{
				lua_pushlightuserdata(L, val);
				return 1;
			}
			return -1;
		}
	};

	template <>
	struct value_getter<lua_CFunction>
	{
		static constexpr int stack_count = 1;

		static bool test(lua_State *L, int idx) noexcept
		{
			return lua_iscfunction(L, idx) ? true : false;
		}

		static lua_CFunction get(lua_State *L, int idx) noexcept
		{
			return lua_tocfunction(L, idx);
		}
	};

	template <>
	struct value_pusher<lua_CFunction>
	{
		static int push(lua_State *L, lua_CFunction val) noexcept
		{
			if (val)
			{
				lua_pushcfunction(L, val);
				return 1;
			}
			return -1;
		}
	};

	template <>
	struct value_getter<lua_State*>
	{
		static constexpr int stack_count = 1;

		static bool test(lua_State *L, int idx) noexcept
		{
			return (lua_type(L, idx) == LUA_TTHREAD);
		}

		static lua_State* get(lua_State *L, int idx) noexcept
		{
			return lua_tothread(L, idx);
		}
	};

	template <>
	struct value_getter<const char*>
	{
		static constexpr int stack_count = 1;

		static bool test(lua_State *L, int idx) noexcept
		{
			return (lua_type(L, idx) == LUA_TSTRING);
		}

		static const char* get(lua_State *L, int idx) noexcept
		{
			return lua_tostring(L, idx);
		}
	};
	
	template <>
	struct value_pusher<const char*>
	{
		static int push(lua_State *L, const char* val) noexcept
		{
			if (val)
			{
				lua_pushstring(L, val);
				return 1;
			}
			return -1;
		}
	};

	template <>
	struct value_pusher<char*>
	{
		static int push(lua_State *L, char* val) noexcept
		{
			if (val)
			{
				lua_pushstring(L, val);
				return 1;
			}
			return -1;
		}
	};

	template <>
	struct value_getter<std::tuple<>>
	{
		static constexpr int stack_count = 0;

		static bool test(lua_State *L, int idx) noexcept
		{
			return true;
		}

		static std::tuple<> get(lua_State *L, int idx) noexcept
		{
			return std::tuple<>();
		}
	};

	template <class _This, class... _Rest>
	struct value_getter<std::tuple<_This, _Rest...>>
	{
		static constexpr int stack_count = param_getter<_This>::stack_count + param_getter<std::tuple<_Rest...>>::stack_count;

		static bool test(lua_State *L, int idx) noexcept
		{
			if (param_getter<_This>::test(L, idx))
			{
				return param_getter<std::tuple<_Rest...>>::test(L, idx + param_getter<_This>::stack_count);
			}
			return false;
		}

		static std::tuple<_This, _Rest...> get(
			lua_State *L, int idx) noexcept
		{
			return std::tuple_cat(std::make_tuple(param_getter<_This>::get(L, idx)),
				param_getter<std::tuple<_Rest...>>::get(L, idx + param_getter<_This>::stack_count));
		}
	};

	template <int idx, int size, class... _Rest>
	struct tuple_pusher
	{
		static int push(lua_State *L, std::tuple<_Rest...> tuple) noexcept
		{
			auto val = std::get<idx>(tuple);
			int i = param_pusher<decltype(val)>::push(L, val);
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

	template <class... _Rest>
	struct value_pusher<std::tuple<_Rest...>>
	{
		static int push(lua_State *L, std::tuple<_Rest...> val) noexcept
		{

			return tuple_pusher<0, sizeof...(_Rest), _Rest...>::push(L, val);
		}
	};*/

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
