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
		if (num_params < 0)
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
}
