////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   environment.h
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

#include <vtd/smart_ptr.h>

namespace luabind
{
	struct env : vtd::ref_obj
	{
		lua_State* L = nullptr;

		inline static int gc(lua_State* L) noexcept
		{
			env* e = *(env**)lua_touserdata(L, 1);
			e->L = nullptr;
			e->dec();
			return 0;
		}
	};

	typedef vtd::smart_ptr<env> env_ptr;

	inline env* get_env(lua_State* L) noexcept
	{
		holder h(L);
		lua_pushglobaltable(L);
		if (lua_getmetatable(L, -1))
		{
			lua_pushstring(L, "__luabind_env");
			lua_rawget(L, -2);
			return *(env**)lua_touserdata(L, -1);
		}
		else
		{
			lua_newtable(L);										//metatable for global
			lua_pushstring(L, "__luabind_env");
			env* e = new env();										//create env
			e->inc();
			e->L = L;
			void* pvData = lua_newuserdata(L, sizeof(env*));		//create env user data
			*(env**)pvData = e;
			lua_newtable(L);										//create metatable for env user data
			lua_pushstring(L, "__gc");
			lua_pushcfunction(L, &env::gc);
			lua_rawset(L, -3);
			lua_setmetatable(L, -2);								//set metatable for env user data
			lua_rawset(L, -3);										//set env user data to __luabind_env
			lua_setmetatable(L, -2);								//set metatable for global
			return e;
		}
	}
}
