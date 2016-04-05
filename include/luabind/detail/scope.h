////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus_test
//  File name:   scope.h
//  Created:     2016/04/05 by Albert D Yang
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

#ifndef LB_SCOPE_INDEX
#define LB_SCOPE_INDEX (1)
#endif

namespace luabind
{
	struct scope;

	namespace detail
	{
		static int key_protection(lua_State* L) noexcept
		{
			lua_pushvalue(L, lua_upvalueindex(1));
			lua_pushvalue(L, -3);
			if (lua_gettable(L, -5) > 1)
			{
				lua_getglobal(L, "tostring");
				lua_pushvalue(L, 2);
				lua_call(L, 1, 1);
				size_t l;
				const char* s = lua_tolstring(L, -1, &l);
				if (s == nullptr)
					return luaL_error(L, "'tostring' must return a string to 'print'");
				char before[] = "new index \"";
				lua_writestring(before, sizeof(before) - 1);
				lua_writestring(s, l);
				char after[] = "\" causing a luabind name conflict.";
				lua_writestring(after, sizeof(after) - 1);
				lua_writeline();
			}
			else
			{
				lua_settop(L, 3);
				lua_rawset(L, -3);
			}
			return 0;
		}

		struct enrollment
		{
			enrollment() noexcept = default;

			virtual ~enrollment() noexcept
			{
				if (next)
				{
					delete next;
					next = nullptr;
				}
			}

		protected:
			virtual void enroll(lua_State*) const noexcept = 0;

		private:
			friend struct scope;
			enrollment* next = nullptr;
		};
	}

	struct scope
	{
		enum Type
		{
			TYPE_NAMESPACE,
			TYPE_MAX
		};

		scope() noexcept = default;

		explicit scope(detail::enrollment* e) noexcept
			: chain(e)
		{

		}

		scope(const scope& copy) noexcept
		{
			chain = copy.chain;
			const_cast<scope&>(copy).chain = nullptr;
		}

		scope(scope&& move) noexcept
		{
			chain = move.chain;
			move.chain = nullptr;
		}

		~scope() noexcept
		{
			if (chain)
			{
				delete chain;
				chain = nullptr;
			}
		}

		scope& operator = (const scope& copy) noexcept
		{
			if (chain)
			{
				delete chain;
				chain = nullptr;
			}
			chain = copy.chain;
			const_cast<scope&>(copy).chain = nullptr;
			return *this;
		}

		scope& operator = (scope&& move) noexcept
		{
			if (chain)
			{
				delete chain;
				chain = nullptr;
			}
			chain = move.chain;
			move.chain = nullptr;
			return *this;
		}		

		scope& operator , (scope s) noexcept
		{
			detail::enrollment*& cur = chain;
			while (cur)
			{
				cur = cur->next;
			}
			cur = s.chain;
			s.chain = nullptr;
			return *this;
		}

		void enroll(lua_State* L) const noexcept
		{
			for (detail::enrollment* r = chain; r != 0; r = r->next)
			{
				LUABIND_CHECK_STACK(L);
				r->enroll(L);
			}
		}

	protected:
		detail::enrollment* chain = nullptr;
	};

	class name_space : public scope
	{
	public:
		struct enrollment : detail::enrollment
		{
			enrollment(const char* n) noexcept
				: name(n)
			{

			}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_CHECK_STACK(L);
				lua_pushstring(L, name);
				if (!lua_rawget(L, -2))
				{
					lua_pop(L, 1);
					lua_newtable(L);					
					lua_pushstring(L, name);
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);				
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
					lua_pushinteger(L, scope::TYPE_NAMESPACE);
					lua_rawseti(L, -2, LB_SCOPE_INDEX);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");
					lua_pushvalue(L, -2);
					lua_pushcclosure(L, &detail::key_protection, 1);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}				
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifdef _DEBUG
				lua_rawgeti(L, -2, LB_SCOPE_INDEX);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == scope::TYPE_NAMESPACE);
				lua_pop(L, 1);
#				endif
				inner_scope.enroll(L);
				lua_pop(L, 3);
			}

			const char* name;
			scope inner_scope;
		};


		explicit name_space(const char* name) noexcept
			: scope(new enrollment(name))
		{

		}

		name_space& operator [] (scope s) noexcept
		{
			((enrollment*)chain)->inner_scope.operator,(s);
			return *this;
		}
	};

	class module_class
	{
	public:
		module_class(lua_State* L, const char* n) noexcept
			: inner(get_env(L)), name(n)
		{

		}

		void operator [] (scope s) noexcept
		{
			if (inner && inner->L)
			{
				
				lua_State* L = inner->L;
				LB_ASSERT(!lua_gettop(L));
				LUABIND_CHECK_STACK(L);
				
				lua_pushglobaltable(L);
				LB_ASSERT_EQ(lua_getmetatable(L, -1), 1);
				lua_pushstring(L, "__index");
				if (!lua_rawget(L, -2))
				{
					lua_pop(L, 1);
					lua_pushinteger(L, scope::TYPE_NAMESPACE);
					lua_rawseti(L, -2, LB_SCOPE_INDEX);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");
					lua_pushvalue(L, -2);
					lua_pushcclosure(L, &detail::key_protection, 1);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifdef _DEBUG
				lua_rawgeti(L, -2, LB_SCOPE_INDEX);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == scope::TYPE_NAMESPACE);
				lua_pop(L, 1);
#				endif
				if (name)
				{
					lua_pushstring(L, name);
					if (!lua_rawget(L, -2))
					{
						lua_pop(L, 1);
						lua_newtable(L);
						lua_pushstring(L, name);
						lua_pushvalue(L, -2);
						lua_rawset(L, -4);
					}
					LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
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
						lua_pushinteger(L, scope::TYPE_NAMESPACE);
						lua_rawseti(L, -2, LB_SCOPE_INDEX);
						lua_newtable(L);
						lua_pushstring(L, "__newindex");
						lua_pushvalue(L, -2);
						lua_pushcclosure(L, &detail::key_protection, 1);
						lua_rawset(L, -4);
						lua_pushstring(L, "__index");
						lua_pushvalue(L, -2);
						lua_rawset(L, -4);
					}
					LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#					ifdef _DEBUG
					lua_rawgeti(L, -2, LB_SCOPE_INDEX);
					LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
						&& lua_tointeger(L, -1) == scope::TYPE_NAMESPACE);
					lua_pop(L, 1);
#					endif
					s.enroll(L);
					lua_pop(L, 3);
				}
				else
				{
					s.enroll(L);
				}
				lua_pop(L, 3);

			}
		}

	private:
		env_ptr inner;
		lua_State* L;
		const char* name;
	};

	inline module_class module(lua_State* L, char const* name = nullptr)
	{
		return module_class(L, name);
	}
}

#ifndef LB_SCOPE_INDEX
#define LB_SCOPE_INDEX (2)
#endif
