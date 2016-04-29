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

#include <vector>
#include <unordered_map>
//#include <vtd/intrusive_ptr.h>

namespace luabind
{
	namespace detail
	{
		struct class_info_data
		{
			typedef std::unordered_map<int, std::pair<ptrdiff_t, class_info_data*>> map;

			int type_id = 0;
			int class_id = 0;
			map base_map;
			map sub_map;
		};

		template<class _Type>
		struct class_info
		{
			static std::unordered_map<lua_State*, class_info_data> info_data_map;
		};

		template<class _Type> std::unordered_map<lua_State*, class_info_data> class_info<_Type>::info_data_map;
	}

	enum related_index
	{
		INDEX_NOP,
		INDEX_SCOPE,
		INDEX_SCOPE_NAME,
		INDEX_FUNC,
		INDEX_READER,
		INDEX_WRITER,
		INDEX_CLASS,
		INDEX_CONSTRUCTOR,
		INDEX_NEW_CONSTRUCTOR,
		INDEX_MAX
	};

	struct env
	{		
		lua_State* L = nullptr;
		std::vector<detail::class_info_data*> class_map;

		void inc() noexcept
		{
			++ref_count;
		}

		void dec() noexcept
		{
			--ref_count;
			if (!ref_count)
			{
				delete_this();
			}
		}

		static int __gc(lua_State* L) noexcept
		{
			env* e = *(env**)lua_touserdata(L, 1);
			for (auto info : e->class_map)
			{
				info->type_id = 0;
				info->class_id = 0;
				info->base_map.clear();
			}
			e->L = nullptr;
			e->dec();
			return 0;
		}

	private:
		virtual void delete_this() noexcept
		{
			delete this;
		}

		size_t ref_count = 0;

	};

	inline lua_State* get_main(lua_State* L) noexcept;

	inline env* get_env(lua_State* L) noexcept
	{
#		if (LUA_VERSION_NUM >= 502)
		L = get_main(L);
#		endif
		LUABIND_HOLD_STACK(L);
#		if (LUA_VERSION_NUM >= 502)
		lua_pushglobaltable(L);
#		else
		lua_pushvalue(L, LUA_GLOBALSINDEX);
#		endif
		if (lua_getmetatable(L, -1))
		{
			lua_rawgeti(L, -1, INDEX_MAX);
			LB_ASSERT(lua_type(L, -1) == LUA_TUSERDATA);
			return *(env**)lua_touserdata(L, -1);
		}
		else
		{
			lua_newtable(L);										//metatable for global
			env* e = new env();										//create env
			e->inc();
			e->L = L;
			void* data = lua_newuserdata(L, sizeof(env*));			//create env user data
			*(env**)data = e;
			lua_newtable(L);										//create metatable for env user data
			lua_pushstring(L, "__gc");
			lua_pushcfunction(L, &env::__gc);
			lua_rawset(L, -3);
			lua_setmetatable(L, -2);								//set metatable for env user data
			lua_rawseti(L, -2, INDEX_MAX);							//set env user data to [INDEX_MAX]
			lua_setmetatable(L, -2);								//set metatable for global
			return e;
		}
	}

	inline lua_State* get_main(lua_State* L) noexcept
	{
#		if (LUA_VERSION_NUM >= 502)
		LUABIND_HOLD_STACK(L);
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
		LB_ASSERT(lua_type(L, -1) == LUA_TTHREAD);
		return lua_tothread(L, -1);
#		else
		return get_env(L)->L;
#		endif
	}
}
