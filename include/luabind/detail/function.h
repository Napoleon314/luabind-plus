////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   function.h
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

#include <functional>
#include <vtd/string.h>

namespace luabind
{
	inline int push_func_name(lua_State* L, const char* s) noexcept
	{
		if (!s) return 0;
		int top = lua_gettop(L);
		lua_pushglobaltable(L);
		const char* start = s;
		const char* end = vtd::strchr(start, '.');
		while (start)
		{
			if (lua_type(L, -1) != LUA_TTABLE)
			{
				lua_pushnil(L);
				break;
			}
			if (end)
			{
				lua_pushlstring(L, start, end - start);
				start = end + 1;
				end = vtd::strchr(start, '.');
			}
			else
			{
				lua_pushstring(L, start);
				start = nullptr;
			}
			lua_gettable(L, -2);			
		}
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			lua_replace(L, top + 1);
			lua_settop(L, top + 1);
			return 1;
		}
		else
		{
			lua_settop(L, top);
			return -1;
		}
	}
	
	template <class _Ret = void, class... _Types>
	_Ret call_function(lua_State* L, const char* func,
		_Types... pak) noexcept
	{
		LUABIND_HOLD_STACK(L);
		if (push_func_name(L, func) != 1)
		{
			LB_LOG_W("%s is not a vaild function", func);
			return type_traits<_Ret>::make_default();
		}
		LB_ASSERT(lua_type(L, -1) == LUA_TFUNCTION);
		int num_params = params_pusher<_Types...>::push(L, pak...);
		if (num_params != params_traits<_Types...>::stack_count)
		{
			LB_LOG_W("call function %s without correct params", func);
			return type_traits<_Ret>::make_default();
		}
		if (lua_pcall(L, num_params, type_traits<_Ret>::stack_count, 0))
		{
			LB_LOG_E("%s", lua_tostring(L, -1));
			return type_traits<_Ret>::make_default();
		}
		if (type_traits<_Ret>::test(L, -type_traits<_Ret>::stack_count))
		{
			return type_traits<_Ret>::get(L, -type_traits<_Ret>::stack_count);
		}
		else
		{
			LB_LOG_E("call function %s with wrong return", func);
			return type_traits<_Ret>::make_default();
		}
	}

	template <int idx, class _Ret, class... _Types>
	struct func_shell
	{
		typedef std::function<_Ret(_Types...)> func_type;
		typedef typename params_trimmer<idx, _Types...>::type val_type;

		static bool test(lua_State* L) noexcept
		{
			return func_invoker<0, 0, idx, _Types...>::test(L, lua_gettop(L));
		}

		func_shell(func_type&& f) noexcept : func(f) {}

		func_type func;
	};

	template <int idx, class _Ret, class... _Types>
	inline func_shell<idx, _Ret, _Types...> create_func_shell(
		std::function<_Ret(_Types...)>&& func) noexcept
	{
		return func_shell<idx, _Ret, _Types...>(std::move(func));
	}

	struct func_holder
	{
		virtual ~func_holder() noexcept
		{
			if (next)
			{
				delete next;
				next = nullptr;
			}
		}

		virtual int call(lua_State* L) noexcept = 0;

		static int __gc(lua_State* L) noexcept
		{
			func_holder* h = *(func_holder**)lua_touserdata(L, 1);
			delete h;
			return 0;
		}

		static int entry(lua_State* L) noexcept
		{
			func_holder* h = *(func_holder**)lua_touserdata(L, lua_upvalueindex(1));
			int ret = h->call(L);
			if (ret < 0)
			{
				return luaL_error(L, "call c++ function with wrong params.");
			}
			else
			{
				return ret;
			}			
		}

		func_holder* next = nullptr;
	};

	template <class _Shell, class... _Types>
	struct func_holder_impl : func_holder
	{
		typedef typename _Shell::func_type func_type;
		typedef typename _Shell::val_type val_type;

		func_holder_impl(const func_type& f, const val_type& v) noexcept
			: func(f), vals(v) {}

		virtual int call(lua_State* L) noexcept
		{
			if (_Shell::test(L))
			{
				return 0;
			}
			else if (next)
			{
				return next->call(L);
			}
			else
			{
				return -1;
			}
		}

		func_type func;
		val_type vals;
	};

	//template <>
	//struct function_enrollment : detail::enrollment
	//{

	//};

	

}
