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

namespace luabind
{
	class handle
	{
	public:
		handle() noexcept = default;

		handle(lua_State* L, int idx) noexcept
		{
			parent = get_env(L);
			if (parent->L)
			{
				lua_pushvalue(parent->L, idx);
				obj_ref = luaL_ref(parent->L, LUA_REGISTRYINDEX);
			}
		}

		handle(const handle& copy) noexcept
		{
			parent = copy.parent;
			if (parent->L)
			{
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, copy.obj_ref);
				obj_ref = luaL_ref(parent->L, LUA_REGISTRYINDEX);
			}
		}

		handle(handle&& move) noexcept
		{
			parent = move.parent;
			obj_ref = move.obj_ref;
			move.parent = nullptr;
			move.obj_ref = 0;
		}

		~handle() noexcept
		{
			clear();
		}

		handle& operator = (const handle& copy) noexcept
		{
			clear();
			parent = copy.parent;
			if (parent->L)
			{
				lua_rawgeti(parent->L, LUA_REGISTRYINDEX, copy.obj_ref);
				obj_ref = luaL_ref(parent->L, LUA_REGISTRYINDEX);
			}
		}

		handle& operator = (handle&& move) noexcept
		{
			clear();
			parent = move.parent;
			obj_ref = move.obj_ref;
			move.parent = nullptr;
			move.obj_ref = 0;
		}

		int push(lua_State* L) const
		{
			if (obj_ref)
			{
				lua_rawgeti(L, LUA_REGISTRYINDEX, obj_ref);
			}
			else
			{
				lua_pushnil(L);
			}
			return 0;
		}

		const env_ptr& get_parent() const
		{
			return parent;
		}

		int get_obj() noexcept
		{
			return obj_ref;
		}

		void replace(lua_State* L, int idx) noexcept
		{
			clear();
			parent = get_env(L);
			if (parent->L)
			{
				lua_pushvalue(parent->L, idx);
				obj_ref = luaL_ref(parent->L, LUA_REGISTRYINDEX);
			}
		}

		void clear() noexcept
		{
			if (obj_ref)
			{
				LB_ASSERT(parent);
				if (parent->L)
				{
					luaL_unref(parent->L, LUA_REGISTRYINDEX, obj_ref);
				}
				parent = nullptr;
				obj_ref = 0;
			}
		}

	private:
		env_ptr parent;
		int obj_ref = 0;
	};

	template <>
	struct can_get_value<handle> : std::true_type
	{

	};

	template <>
	struct can_push_value<handle> : std::true_type
	{

	};

	template <>
	struct value_getter<handle>
	{
		static constexpr int stack_count = 1;

		static bool test(lua_State *L, int idx) noexcept
		{
			return (lua_type(L, idx) != LUA_TNIL);
		}

		static handle get(lua_State *L, int idx) noexcept
		{			 
			return std::move(handle(L, idx));
		}
	};

	template <>
	struct value_pusher<handle>
	{
		static int push(lua_State *L, handle val) noexcept
		{
			return val.push(L);
		}
	};


}
