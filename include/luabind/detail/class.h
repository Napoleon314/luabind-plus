////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   class.h
//  Created:     2016/04/10 by Albert D Yang
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

#include <vtd/rtti.h>

namespace luabind
{
	template<class _Der, class... _Bases>
	struct base_finder;

	template<class _Der>
	struct base_finder<_Der>
	{
		static void find(env& e, std::vector<std::pair<ptrdiff_t, detail::class_info_data*>>& vec) noexcept
		{

		}
	};

	template<class _Der, class _This, class... _Rest>
	struct base_finder<_Der, _This, _Rest...>
	{
		static void find(env& e, std::vector<std::pair<ptrdiff_t, detail::class_info_data*>>& vec) noexcept
		{
			detail::class_info_data* info = &(detail::class_info<_Der>::info_data_map[e.L]);
			if (info->type_id && info->class_id)
			{
				ptrdiff_t diff = vtd::rtti::base::offset<_This, _Der>();
				vec.push_back(std::make_pair(diff, info));
				base_finder<_Der, _Rest...>::find(e, vec);
			}
		}
	};

	template<class _Der, class... _Bases>
	class class_ : public scope
	{
	public:
		static int __tostring(lua_State* L) noexcept
		{
			char buf[LB_BUF_SIZE];
			const char* name = lua_tostring(L, lua_upvalueindex(1));
			sprintf(buf, "c++ class[%s]", name);
			lua_pushstring(L, buf);
			return 1;
		}

		static int __newindex(lua_State* L) noexcept
		{
			if (lua_getmetatable(L, 1))
			{
				if (lua_rawgeti(L, -1, INDEX_WRITER) == LUA_TTABLE)
				{
					lua_pushvalue(L, 2);
					if (lua_rawget(L, -2) == LUA_TFUNCTION)
					{
						lua_pushvalue(L, 3);
						if (lua_pcall(L, 1, 0, 0))
						{
							return luaL_error(L, lua_tostring(L, -1));
						}
						else
						{
							return 0;
						}
					}
				}
				lua_settop(L, 3);
			}
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, 3);
			lua_call(L, 1, 1);
			const char* s = lua_tostring(L, -1);
			const char* scope_name = lua_tostring(L, lua_upvalueindex(1));
			const char* name = lua_tostring(L, 2);
			return luaL_error(L, "\"%s.%s = %s\" try to modify a inexistent value in a c++ class.", scope_name, name, s);
		}

		struct enrollment : detail::enrollment
		{
			static detail::class_info_data* get_class_info(lua_State* L) noexcept
			{
				env& e = *get_env(L);
				detail::class_info_data* info = &(detail::class_info<_Der>::info_data_map[e.L]);
				if (!info->type_id)
				{
					info->type_id = int(e.class_map.size() + 1);
					e.class_map[info->type_id] = info;
					info->base_vec.clear();
					base_finder<_Der, _Bases...>::find(e, info->base_vec);
					LB_ASSERT(info->base_vec.size() == (sizeof...(_Bases)));
				}
				return info;
			}

			enrollment(const char* n) noexcept
				: name(n)
			{

			}

			virtual void enroll(lua_State* L) const noexcept
			{
				auto info = get_class_info(L);
				LUABIND_CHECK_STACK(L);
				char full_name[LB_BUF_SIZE];
				LB_ASSERT_EQ(lua_rawgeti(L, -2, INDEX_SCOPE_NAME), LUA_TSTRING);
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
				if (!lua_getmetatable(L, -1))
				{
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_setmetatable(L, -3);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
				lua_pushstring(L, "__index");
				if (!lua_rawget(L, -2))
				{
					lua_pop(L, 1);
					lua_pushinteger(L, SCOPE_CLASS);
					lua_rawseti(L, -2, INDEX_SCOPE);
					lua_pushstring(L, full_name);
					lua_rawseti(L, -2, INDEX_SCOPE_NAME);
					lua_pushstring(L, "__tostring");
					lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &class_::__tostring, 1);
					lua_rawset(L, -3);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");					
					lua_rawgeti(L, -3, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &class_::__newindex, 1);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifndef NDEBUG
				lua_rawgeti(L, -2, INDEX_SCOPE);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == SCOPE_CLASS);
				lua_pop(L, 1);
#				endif
				if (!info->class_id)
				{
					lua_pushvalue(L, -3);
					info->class_id = luaL_ref(L, LUA_REGISTRYINDEX);
				}
				if (lua_rawgeti(L, -2, INDEX_CLASS) != LUA_TTABLE)
				{
					lua_pop(L, 1);
					lua_newtable(L);

					//lua_pushvalue(L, -1);

					//int top = lua_gettop(L);
					//top = lua_gettop(L);

				}
				int top = lua_gettop(L);
				member_scope.enroll(L);
				lua_pop(L, 1);
				inner_scope.enroll(L);
				lua_pop(L, 3);
			}

			const char* name;
			scope member_scope;
			scope inner_scope;			
		};

		explicit class_(const char* name) noexcept
			: scope(new enrollment(name))
		{
			
		}

		class_& operator [] (scope s) noexcept
		{
			((enrollment*)chain)->inner_scope.operator,(s);
			return *this;
		}

	};


}
