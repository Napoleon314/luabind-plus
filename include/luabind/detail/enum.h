////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   enum.h
//  Created:     2016/04/24 by Albert D Yang
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

namespace luabind
{
	class enum_ : public scope
	{
	public:
		static int __tostring(lua_State* L) noexcept
		{
			char buf[LB_BUF_SIZE];
			const char* name = lua_tostring(L, lua_upvalueindex(1));
			sprintf(buf, "c++ enum[%s]", name);
			lua_pushstring(L, buf);
			return 1;
		}

		static int __newindex(lua_State* L) noexcept
		{
			const char* name = lua_tostring(L, lua_upvalueindex(1));
			return luaL_error(L, "can not modify c++ enum[%s].", name);
		}

		struct value : detail::enrollment
		{
			value(const char* i, const char* o, int v) noexcept
				: inner(i), outer(o), val(v)
			{
				if (!inner) inner = outer;
			}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_CHECK_STACK(L);
				if (outer)
				{
					lua_pushstring(L, outer);
					lua_pushinteger(L, val);
					lua_rawset(L, -6);
				}
				if (inner)
				{
					lua_pushstring(L, inner);
					lua_pushinteger(L, val);
					lua_rawset(L, -3);
				}
			}

			const char* inner;
			const char* outer;
			int val;
		};

		struct enrollment : detail::enrollment
		{
			enrollment(const char* n) noexcept
				: name(n)
			{

			}

			static void getmetatable(lua_State* L, const char* full_name) noexcept
			{
				if (!lua_getmetatable(L, -1))
				{
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_setmetatable(L, -3);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
				lua_pushstring(L, "__index");
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) != LUA_TTABLE)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif
				{
					lua_pop(L, 1);
					lua_pushinteger(L, SCOPE_ENUM);
					lua_rawseti(L, -2, INDEX_SCOPE);
					lua_pushstring(L, full_name);
					lua_rawseti(L, -2, INDEX_SCOPE_NAME);
					lua_pushstring(L, "__tostring");
					lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &__tostring, 1);
					lua_rawset(L, -3);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");
					lua_rawgeti(L, -3, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &__newindex, 1);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifndef NDEBUG
				lua_rawgeti(L, -2, INDEX_SCOPE);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == SCOPE_ENUM);
				lua_pop(L, 1);
#				endif
			}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_CHECK_STACK(L);
				char full_name[LB_BUF_SIZE];
				lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
				LB_ASSERT(lua_type(L, -1) == LUA_TSTRING);
				const char* super_name = lua_tostring(L, -1);
				if (*super_name)
				{
					sprintf(full_name, "%s.%s", super_name, name);
				}
				else
				{
					sprintf(full_name, "%s", name);
				}
				lua_pop(L, 1);
				gettable(L, name);
				getmetatable(L, full_name);
				inner_scope.enroll(L);
				lua_pop(L, 3);
			}

			const char* name;
			scope inner_scope;
		};

		explicit enum_(const char* name) noexcept
			: scope(new enrollment(name))
		{

		}

		enum_& def(const char* outer, int val, const char* inner = nullptr) noexcept
		{
			((enrollment*)chain)->inner_scope.operator,
				(scope(new value(inner, outer, val)));
			return *this;
		}

	};
}
