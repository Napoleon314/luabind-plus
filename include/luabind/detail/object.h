////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   object.h
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

namespace luabind
{
	class object
	{
	public:
		object() noexcept = default;

		object(lua_State* L, int idx) noexcept
		{
			replace(L, idx);
		}

		object(const object& copy) noexcept
		{
			replace(copy);
		}

		object(object&& move) noexcept
		{
			replace(move);
		}

		~object() noexcept
		{
			clear();
		}

		object& operator = (const object& copy) noexcept
		{
			replace(copy);
			return *this;
		}

		object& operator = (object&& move) noexcept
		{
			replace(move);
			return *this;
		}

		lua_State* get_lua() noexcept
		{
			return parent ? parent->L : nullptr;
		}

		int push(lua_State* L) const
		{
			if (handle)
			{
				lua_rawgeti(L, LUA_REGISTRYINDEX, handle);
				return 1;
			}
			else
			{
				return 0;
			}
		}

		const env_ptr& get_parent() const noexcept
		{
			return parent;
		}

		int get_handle() const noexcept
		{
			return handle;
		}
		
		void replace(lua_State* L, int idx) noexcept
		{
			clear();
			parent = get_env(L);
			if (parent->L)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_pushvalue(parent->L, idx);
				if (lua_type(parent->L, -1) == LUA_TNONE)
				{
					handle = 0;
				}
				else
				{
					handle = luaL_ref(parent->L, LUA_REGISTRYINDEX);
				}				
			}
		}

		void replace(const object& copy) noexcept
		{
			clear();
			parent = copy.parent;
			if (parent->L && copy.handle)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, copy.handle);
				handle = luaL_ref(parent->L, LUA_REGISTRYINDEX);
			}
			else
			{
				handle = 0;
			}
		}

		void replace(object&& move) noexcept
		{
			clear();
			parent = move.parent;
			handle = move.handle;
			move.parent = nullptr;
			move.handle = 0;
		}

		template <class _Ty>
		void set(lua_State* L, _Ty val) noexcept
		{			
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			clear();
			parent = get_env(L);
			if (parent->L)
			{
				LUABIND_HOLD_STACK(L);
				int n = type_traits<_Ty>::push(L, val);
				if (!n)
				{
					handle = 0;
				}
				else
				{
					LB_ASSERT(n == 1);
					handle = luaL_ref(parent->L, LUA_REGISTRYINDEX);
				}
			}
		}

		template <class _Ty>
		_Ty get() noexcept
		{
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			if (parent && parent->L && handle)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (type_traits<_Ty>::test(parent->L, -1))
				{
					return type_traits<_Ty>::get(parent->L, -1);
				}
			}
			return type_traits<_Ty>::make_default();
		}

		int get_type() const noexcept
		{
			if (parent && parent->L && handle)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				return lua_type(parent->L, -1);
			}
			return LUA_TNONE;
		}

		bool is_table() const noexcept
		{
			return get_type() == LUA_TTABLE;
		}

		bool is_valid() const noexcept
		{
			return parent && parent->L;
		}

		void clear() noexcept
		{
			if (handle)
			{
				LB_ASSERT(parent);
				if (parent->L)
				{
					luaL_unref(parent->L, LUA_REGISTRYINDEX, handle);
				}
				parent = nullptr;
				handle = 0;
			}
		}

		template <class _Val, class _Key>
		_Val gettable(_Key key) noexcept
		{
			static_assert(type_traits<_Val>::stack_count == 1
				&& type_traits<_Key>::stack_count == 1,
				"_Val and _Key have to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (type_traits<_Key>::push(parent->L, key) == 1)
				{
					lua_gettable(parent->L, -2);
					if (type_traits<_Val>::test(parent->L, -1))
					{
						return type_traits<_Val>::get(parent->L, -1);
					}
				}				
			}
			return type_traits<_Val>::make_default();
		}

		template <class _Val, class _Key>
		void settable(_Key key, _Val val) noexcept
		{
			static_assert(type_traits<_Val>::stack_count == 1
				&& type_traits<_Key>::stack_count == 1,
				"_Val and _Key have to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (type_traits<_Key>::push(parent->L, key) == 1)
				{
					if (type_traits<_Val>::push(parent->L, val) == 1)
					{
						lua_settable(parent->L, -3);
					}
				}				
			}
		}

		object rawget(const char* key) noexcept
		{
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_pushstring(parent->L, key);
				lua_rawget(parent->L, -2);
				return object(parent->L, -1);
			}
			return object();
		}

		object rawgeti(int key) noexcept
		{
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_rawgeti(parent->L, -1, key);
				return object(parent->L, -1);
			}
			return object();
		}

		template <class _Ty>
		_Ty rawget(const char* key) noexcept
		{
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_pushstring(parent->L, key);
				lua_rawget(parent->L, -2);
				if (type_traits<_Ty>::test(parent->L, -1))
				{
					return type_traits<_Ty>::get(parent->L, -1);
				}
			}
			return type_traits<_Ty>::make_default();
		}

		template <class _Ty>
		_Ty rawgeti(int key) noexcept
		{
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_rawgeti(parent->L, -1, key);
				if (type_traits<_Ty>::test(parent->L, -1))
				{
					return type_traits<_Ty>::get(parent->L, -1);
				}
			}
			return type_traits<_Ty>::make_default();
		}

		void rawset(const char* key, object obj) noexcept
		{
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_pushstring(parent->L, key);
				if (obj.push(parent->L) == 1)
				{
					lua_rawset(parent->L, -3);
				}
			}
		}

		void rawseti(int key, object obj) noexcept
		{
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (obj.push(parent->L) == 1)
				{
					lua_rawseti(parent->L, -2, key);
				}
			}
		}

		template <class _Ty>
		void rawset(const char* key, _Ty val) noexcept
		{
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_pushstring(parent->L, key);
				if (type_traits<_Ty>::push(parent->L, val) == 1)
				{
					lua_rawset(parent->L, -3);
				}
			}
		}

		template <class _Ty>
		void rawseti(int key, _Ty val) noexcept
		{
			static_assert(type_traits<_Ty>::stack_count == 1,
				"_Ty has to occupy 1 stack");
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (type_traits<_Ty>::push(parent->L, val) == 1)
				{
					lua_rawseti(parent->L, -2, key);
				}
			}
		}

		object getmetatable() noexcept
		{
			if (is_table())
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				if (lua_getmetatable(parent->L, -1))
				{
					return object(parent->L, -1);
				}
			}
			return object();
		}

		void setmetatable(object obj) noexcept
		{
			switch (get_type())
			{
			case LUA_TTABLE:
			case LUA_TUSERDATA:
				if (obj.is_table())
				{
					LUABIND_HOLD_STACK(parent->L);
					lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
					if (obj.push(parent->L) == 1)
					{
						lua_setmetatable(parent->L, -2);
					}
				}
				break;
			default:
				break;
			}
		}

#		if (LUA_VERSION_NUM >= 502)
		size_t rawlen() const noexcept
		{
			if (parent && parent->L && handle)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				return lua_rawlen(parent->L, -1);
			}
			return 0;
		}
#		endif

		void foreach(std::function<void(lua_State* L)> func) noexcept
		{
			if (parent && parent->L && handle)
			{
				LUABIND_HOLD_STACK(parent->L);
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, handle);
				lua_pushnil(parent->L);
				while (lua_next(parent->L, -2) != 0)
				{
					func(parent->L);
					lua_pop(parent->L, 1);
				}
			}
		}

		template <class _Ty>
		operator _Ty () noexcept
		{
			return get<_Ty>();
		}

		object operator [] (const char* key) noexcept
		{
			return gettable<object>(key);
		}

		object operator [] (int key) noexcept
		{
			return gettable<object,int>(key);
		}

	private:
		env_ptr parent;
		int handle = 0;
	};

	template <>
	struct type_traits<object>
	{
		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return (lua_type(L, idx) != LUA_TNONE);
		}

		static object get(lua_State* L, int idx) noexcept
		{
			return object(L, idx);
		}

		static int push(lua_State* L, object val) noexcept
		{
			return val.push(L);
		}

		static object make_default() noexcept
		{
			return object();
		}
	};

	inline object newtable(lua_State* L) noexcept
	{
		LUABIND_HOLD_STACK(L);
		lua_newtable(L);
		return object(L, -1);
	}

	inline object globals(lua_State* L) noexcept
	{
		LUABIND_HOLD_STACK(L);
#		if (LUA_VERSION_NUM >= 502)
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
#		else
		lua_pushvalue(L, LUA_GLOBALSINDEX);
#		endif
		return object(L, -1);
	}

#	if (LUA_VERSION_NUM >= 502)
	inline object mainthread(lua_State* L) noexcept
	{
		LUABIND_HOLD_STACK(L);
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
		return object(L, -1);
	}
#	endif
}
