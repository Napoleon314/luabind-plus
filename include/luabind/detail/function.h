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
		typedef _Ret ret_type;
		typedef std::tuple<_Types...> tuple;

		static constexpr int params_count = sizeof...(_Types);
		static constexpr int default_start = idx;

		static bool test(lua_State* L, int top) noexcept
		{
			return func_tester<0, 0, idx, _Types...>::test(L, top);
		}

		static bool construct_test(lua_State* L, int top) noexcept
		{
			return construct_tester<idx, _Types...>::test(L, top);
		}

		func_shell(func_type&& f) noexcept : func(f) {}

		func_type func;
	};

	template <int idx, class _Ret, class... _Types>
	func_shell<idx, _Ret, _Types...> create_func_shell(
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
				return luaL_error(L, "call c++ function[%s.%s] with wrong params.",
					lua_tostring(L, lua_upvalueindex(2)),
					lua_tostring(L, lua_upvalueindex(3)));
			}
			else
			{
				return ret;
			}			
		}

		static int construct_entry(lua_State* L) noexcept
		{
			func_holder* h = *(func_holder**)lua_touserdata(L, lua_upvalueindex(1));
			int ret = h->call(L);
			if (ret < 0)
			{
				return luaL_error(L, "construct c++ class[%s] with wrong params.",
					lua_tostring(L, lua_upvalueindex(2)));
			}
			else
			{
				return ret;
			}
		}

		func_holder* next = nullptr;
	};

	template <class _Shell>
	struct do_invoke_normal
	{
		static int invoke(typename _Shell::func_type& func, typename _Shell::val_type& vals,
			lua_State* L, int top) noexcept
		{
			return type_traits<typename _Shell::ret_type>::push(L,
				func_invoker<0, _Shell::default_start, _Shell>::invoke(
					func, vals, L, top));
		}
	};

	template <class _Shell>
	struct do_invoke_no_return
	{
		static int invoke(typename _Shell::func_type& func, typename _Shell::val_type& vals,
			lua_State* L, int top) noexcept
		{
			func_invoker<0, _Shell::default_start, _Shell>::invoke(func, vals, L, top);
			return 0;
		}
	};

	template <class _Shell>
	struct do_invoke : std::conditional < std::is_same<typename _Shell::ret_type, void>::value,
		do_invoke_no_return<_Shell>, do_invoke_normal < _Shell >> ::type
	{

	};

	template <class _Shell>
	struct func_holder_impl : func_holder
	{
		typedef typename _Shell::func_type func_type;
		typedef typename _Shell::val_type val_type;

		func_holder_impl(const func_type& f, const val_type& v) noexcept
			: func(f), vals(v) {}

		virtual int call(lua_State* L) noexcept
		{
			int top = lua_gettop(L);
			if (_Shell::test(L, top))
			{				
				return do_invoke<_Shell>::invoke(func, vals, L, top);			
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
}
