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

#include <memory>
#include <vtd/rtti.h>

namespace luabind
{
	enum userdata_type
	{
		USERDATA_CLASS,
		USERDATA_CUSTOMIZED_BEGIN
	};

	enum storage_type
	{
		STORAGE_LUA,
		STORAGE_I_PTR,
		STORAGE_U_PTR,
		STORAGE_S_PTR,
		STORAGE_W_PTR,
		STORAGE_MAX
	};

	enum obj_inner_index
	{
		OBJ_NOP,
		OBJ_INDEX,
		OBJ_NEW_INDEX,
		OBJ_FUNC_HOLDER,
		OBJ_FUNC,		
		OBJ_READER,
		OBJ_WRITER,
		OBJ_SUPER,
		OBJ_MAX
	};

	template <class... _Types>
	struct constructor
	{
		template <class _Der>
		static void default_construct_func(void* m, _Types... pak) noexcept
		{
			new(m) _Der(pak...);
		}

		template <class _Der>
		static _Der* default_new_func(_Types... pak) noexcept
		{
			return new _Der(pak...);
		}
	};

	template <int idx, class _Der, class _Ret, class... _Types>
	struct member_func_shell
	{
		typedef _Der _Class;
		typedef _Ret(_Der::*func_type)(_Types...);
		typedef typename params_trimmer<idx, _Types...>::type val_type;
		typedef _Ret ret_type;
		typedef std::tuple<_Types...> tuple;

		static constexpr int params_count = sizeof...(_Types);
		static constexpr int default_start = idx;

		static bool test(lua_State* L, int top) noexcept
		{
			return func_tester<1, 0, idx, _Types...>::test(L, top);
		}	

		member_func_shell(func_type f) noexcept : func(f) {}

		func_type func;
	};

	template <int idx, class _Der, class _Ret, class... _Types>
	member_func_shell<idx, _Der, _Ret, _Types...> create_member_func_shell(
		_Ret(_Der::*func)(_Types...)) noexcept
	{
		return member_func_shell<idx, _Der, _Ret, _Types...>(func);
	}

	namespace detail
	{
		struct header
		{
			short type;
			short storage;
			int type_id;
		};

		template <class _Ty, storage_type s>
		struct userdata_obj {};

		template <class _Ty>
		struct userdata_obj<_Ty, STORAGE_LUA>
		{
			header info;
			_Ty data;
		};

		template <class _Ty>
		struct userdata_obj<_Ty, STORAGE_I_PTR>
		{
			header info;
			_Ty* data;
		};

		template <class _Ty>
		struct userdata_obj<_Ty, STORAGE_U_PTR>
		{
			header info;
			std::unique_ptr<_Ty> data;
		};

		template <class _Ty>
		struct userdata_obj<_Ty, STORAGE_S_PTR>
		{
			header info;
			std::shared_ptr<_Ty> data;
		};

		template <class _Ty>
		struct userdata_obj<_Ty, STORAGE_W_PTR>
		{
			header info;
			std::weak_ptr<_Ty> data;
		};

		template <class _Der, class _Shell>
		struct constructor_holder : func_holder
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			static_assert(std::is_same<typename _Shell::ret_type, void>::value
				&& std::is_same<typename std::tuple_element<0, typename _Shell::tuple>::type, void*>::value
				&& _Shell::default_start >= 1, "wrong construct function.");

			constructor_holder(const func_type& f, const val_type& v) noexcept
				: func(f), vals(v) {}

			virtual ~constructor_holder() noexcept
			{

			}

			virtual int call(lua_State* L) noexcept
			{				
				int top = lua_gettop(L);
				if (_Shell::construct_test(L, top))
				{
					header* data = (header*)lua_newuserdata(L, sizeof(_Der) + sizeof(header));
					data->type = USERDATA_CLASS;
					data->storage = STORAGE_LUA;					
					data->type_id = detail::class_info<_Der>::info_data_map[get_main(L)].type_id;
					func_invoker<1, _Shell::default_start, _Shell, void*>::invoke(
						func, vals, L, top, data + 1);
					lua_pushvalue(L, lua_upvalueindex(3));
					lua_setmetatable(L, -2);
					return 1;
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

		inline int construct_entry(lua_State* L) noexcept
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

		template <class _Der, class _Shell, class... _Types>
		struct construct_func : enrollment
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			construct_func(_Shell& f, _Types... pak) noexcept
				: func(std::move(f.func)), values(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);

#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -3, INDEX_CONSTRUCTOR) != LUA_TUSERDATA)
#				else
				lua_rawgeti(L, -3, INDEX_CONSTRUCTOR);
				if (lua_type(L, -1) != LUA_TUSERDATA)
#				endif
				{
					lua_pop(L, 1);
					void* data = lua_newuserdata(L, sizeof(func_holder*));
					*(func_holder**)data = new constructor_holder<_Der, _Shell>(func, values);
					lua_newtable(L);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &func_holder::__gc);
					lua_rawset(L, -3);
					lua_setmetatable(L, -2);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -5, INDEX_CONSTRUCTOR);
					lua_pushstring(L, "__call");
					lua_pushvalue(L, -2);					
					lua_rawgeti(L, -6, INDEX_SCOPE_NAME);					
					lua_pushvalue(L, -5);
					lua_pushcclosure(L, &construct_entry, 3);
					lua_rawset(L, -6);
				}
				else
				{
					func_holder* h = *(func_holder**)lua_touserdata(L, -1);
					while (true)
					{
						if (h->next)
						{
							h = h->next;
						}
						else
						{
							h->next = new constructor_holder<_Der, _Shell>(func, values);
							break;
						}
					}
				}
			}

			func_type func;
			val_type values;
		};

		template <class _Der, class _Shell>
		struct new_holder : func_holder
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			static_assert(std::is_same<typename _Shell::ret_type, _Der*>::value,
				"wrong new function.");

			new_holder(const func_type& f, const val_type& v) noexcept
				: func(f), vals(v) {}

			virtual ~new_holder() noexcept
			{

			}

			virtual int call(lua_State* L) noexcept
			{
				int top = lua_gettop(L);
				if (_Shell::test(L, top))
				{
					_Der* p = func_invoker<0, _Shell::default_start, _Shell>::invoke(
						func, vals, L, top);
					if (p)
					{
						storage_type eType = (storage_type)lua_tointeger(L, lua_upvalueindex(3));
						switch (eType)
						{
						case STORAGE_I_PTR:
							return object_traits<_Der*>::push(L, p);
						case STORAGE_U_PTR:
							return object_traits<std::unique_ptr<_Der>>::push(L, std::unique_ptr<_Der>(p));
						case STORAGE_S_PTR:
							return object_traits<std::shared_ptr<_Der>>::push(L, std::shared_ptr<_Der>(p));
						default:
							break;
						}
					}
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

		inline int new_entry(lua_State* L) noexcept
		{
			func_holder* h = *(func_holder**)lua_touserdata(L, lua_upvalueindex(1));
			int ret = h->call(L);
			if (ret < 0)
			{
				return luaL_error(L, "new c++ class[%s] with wrong params.",
					lua_tostring(L, lua_upvalueindex(2)));
			}
			else
			{
				return ret;
			}
		}

		template <class _Der, class _Shell, class... _Types>
		struct new_func : enrollment
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			new_func(_Shell& f, _Types... pak) noexcept
				: func(std::move(f.func)), values(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -3, INDEX_FUNC) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -3, INDEX_FUNC);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -5, INDEX_FUNC);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);				
				lua_pushstring(L, "new");
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) != LUA_TUSERDATA)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) != LUA_TUSERDATA)
#				endif
				{
					lua_pop(L, 1);					
					void* data = lua_newuserdata(L, sizeof(func_holder*));
					*(func_holder**)data = new new_holder<_Der, _Shell>(func, values);
					lua_newtable(L);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &func_holder::__gc);
					lua_rawset(L, -3);
					lua_setmetatable(L, -2);
					lua_pushstring(L, "new");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);					

					lua_pushstring(L, "new");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -7, INDEX_SCOPE_NAME);
					lua_pushinteger(L, STORAGE_I_PTR);
					lua_pushcclosure(L, &new_entry, 3);					
					lua_rawset(L, -6);

					lua_pushstring(L, "new_i");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -7, INDEX_SCOPE_NAME);
					lua_pushinteger(L, STORAGE_I_PTR);
					lua_pushcclosure(L, &new_entry, 3);
					lua_rawset(L, -6);

					lua_pushstring(L, "new_u");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -7, INDEX_SCOPE_NAME);
					lua_pushinteger(L, STORAGE_U_PTR);
					lua_pushcclosure(L, &new_entry, 3);
					lua_rawset(L, -6);

					lua_pushstring(L, "new_s");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -7, INDEX_SCOPE_NAME);
					lua_pushinteger(L, STORAGE_S_PTR);
					lua_pushcclosure(L, &new_entry, 3);
					lua_rawset(L, -6);
				}
				else
				{
					func_holder* h = *(func_holder**)lua_touserdata(L, -1);
					while (true)
					{
						if (h->next)
						{
							h = h->next;
						}
						else
						{
							h->next = new new_holder<_Der, _Shell>(func, values);
							break;
						}
					}
				}
			}

			func_type func;
			val_type values;
		};

		inline void* get_adjusted_ptr(header* data, const class_info_data& info) noexcept
		{
			if (data->type == USERDATA_CLASS)
			{
				void* origin(nullptr);
				switch (data->storage)
				{
				case STORAGE_LUA:
					origin = (data + 1);
					break;
				case STORAGE_I_PTR:
					origin = *(void**)(data + 1);
					break;
				case STORAGE_U_PTR:
					origin = ((std::unique_ptr<void>*)(data + 1))->get();
					break;
				case STORAGE_S_PTR:
					origin = ((std::shared_ptr<void>*)(data + 1))->get();
					break;
				case STORAGE_W_PTR:
					if (!(((std::weak_ptr<void>*)(data + 1))->expired()))
					{
						origin = ((std::weak_ptr<void>*)(data + 1))->lock().get();
					}					
					break;
				default:
					break;
				}
				if (origin)
				{
					if (info.type_id == data->type_id)
					{
						return origin;
					}
					else
					{
						auto it = info.sub_map.find(data->type_id);
						if (it != info.sub_map.end())
						{
							return (char*)origin + it->second.first;
						}
					}
				}
			}
			return nullptr;
		}

		struct member_func_holder
		{
			virtual ~member_func_holder() noexcept
			{
				if (next)
				{
					delete next;
					next = nullptr;
				}
			}

			virtual int call(lua_State* L, void* obj) noexcept = 0;

			static int __gc(lua_State* L) noexcept
			{
				member_func_holder* h = *(member_func_holder**)lua_touserdata(L, 1);
				delete h;
				return 0;
			}

			static int entry(lua_State* L) noexcept
			{				
				if (lua_type(L, 1) == LUA_TUSERDATA)
				{				
					void* ptr = get_adjusted_ptr((header*)lua_touserdata(L, 1),
						*(class_info_data*)lua_touserdata(L, lua_upvalueindex(1)));
					if (ptr)
					{
						member_func_holder* h = *(member_func_holder**)lua_touserdata(L, lua_upvalueindex(2));
						int ret = h->call(L, ptr);
						if (ret < 0)
						{
							return luaL_error(L, "call c++ member function[%s:%s] with wrong params.",
								lua_tostring(L, lua_upvalueindex(3)), lua_tostring(L, lua_upvalueindex(4)));
						}
						else
						{
							return ret;
						}
					}
				}
				return luaL_error(L, "call c++ member function[%s:%s] with invalid object.",
					lua_tostring(L, lua_upvalueindex(3)), lua_tostring(L, lua_upvalueindex(4)));
			}

			member_func_holder* next = nullptr;
		};

		template <int base, class _Shell>
		struct do_obj_invoke_normal
		{
			static int invoke(typename _Shell::_Class* o, typename _Shell::func_type f,
				typename _Shell::val_type& v, lua_State* L, int top) noexcept
			{
				return type_traits<typename _Shell::ret_type>::push(L,
					member_func_invoker<base, _Shell::default_start, _Shell>::invoke(
						o, f, v, L, top));
			}
		};

		template <int base, class _Shell>
		struct do_obj_invoke_no_return
		{
			static int invoke(typename _Shell::_Class* o, typename _Shell::func_type f,
				typename _Shell::val_type& v, lua_State* L, int top) noexcept
			{
				member_func_invoker<base, _Shell::default_start, _Shell>::invoke(
					o, f, v, L, top);
				return 0;
			}
		};

		template <int base, class _Shell>
		struct do_obj_invoke : std::conditional < std::is_same<typename _Shell::ret_type, void>::value,
			do_obj_invoke_no_return<base, _Shell>, do_obj_invoke_normal <base, _Shell>> ::type
		{

		};

		template <class _Shell>
		struct member_func_holder_impl : member_func_holder
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			member_func_holder_impl(func_type f, const val_type& v) noexcept
				: func(f), vals(v) {}

			virtual int call(lua_State* L, void* obj) noexcept
			{
				int top = lua_gettop(L);
				if (_Shell::test(L, top))
				{
					return do_obj_invoke<1, _Shell>::invoke((typename _Shell::_Class*)obj,
						func, vals, L, top);
				}
				else if (next)
				{
					return next->call(L, obj);
				}
				else
				{
					return -1;
				}
			}			

			func_type func;
			val_type vals;
		};

		template <class... _Types>
		struct manual_member_func : enrollment
		{
			typedef std::tuple<_Types...> holder;
			static_assert(type_traits<holder>::can_push, "holder types wrong.");

			manual_member_func(const char* n, lua_CFunction f, _Types... pak) noexcept
				: name(n), func(f), upvalues(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_FUNC) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -1, OBJ_FUNC);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif				
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -3, OBJ_FUNC);				
				}
				lua_pushstring(L, name);
				LB_ASSERT_EQ(type_traits<holder>::push(L, upvalues),
					type_traits<holder>::stack_count);
				lua_pushcclosure(L, func, type_traits<holder>::stack_count);
				lua_rawset(L, -3);
			}

			const char* name;
			lua_CFunction func;
			holder upvalues;
		};

		template <class _Shell, class... _Types>
		struct member_func : enrollment
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			member_func(const char* n, _Shell& f, _Types... pak) noexcept
				: name(n), func(f.func), values(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_FUNC_HOLDER) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -1, OBJ_FUNC_HOLDER);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif				
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -3, OBJ_FUNC_HOLDER);
				}
				lua_pushstring(L, name);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) != LUA_TUSERDATA)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) != LUA_TUSERDATA)
#				endif
				{
					lua_pop(L, 1);
					lua_pushlightuserdata(L, &class_info<typename _Shell::_Class>::info_data_map[get_main(L)]);
					void* data = lua_newuserdata(L, sizeof(func_holder*));
					*(member_func_holder**)data = new member_func_holder_impl<_Shell>(func, values);
					lua_newtable(L);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &member_func_holder::__gc);
					lua_rawset(L, -3);
					lua_setmetatable(L, -2);
					lua_pushstring(L, name);
					lua_pushvalue(L, -2);
					lua_rawset(L, -5);
					
					lua_rawgeti(L, -6, INDEX_SCOPE_NAME);
					lua_pushstring(L, name);
					lua_pushcclosure(L, &member_func_holder::entry, 4);
#					if (LUA_VERSION_NUM >= 503)
					if (lua_rawgeti(L, -3, OBJ_FUNC) != LUA_TTABLE)
#					else
					lua_rawgeti(L, -3, OBJ_FUNC);
					if (lua_type(L, -1) != LUA_TTABLE)
#					endif
					{
						lua_pop(L, 1);
						lua_newtable(L);
						lua_pushvalue(L, -1);
						lua_rawseti(L, -5, OBJ_FUNC);
					}
					lua_pushstring(L, name);
					lua_pushvalue(L, -3);
					lua_rawset(L, -3);
				}
				else
				{
					member_func_holder* h = *(member_func_holder**)lua_touserdata(L, -1);
					while (true)
					{
						if (h->next)
						{
							h = h->next;
						}
						else
						{
							h->next = new member_func_holder_impl<_Shell>(func, values);
							break;
						}
					}
				}
			}

			const char* name;
			func_type func;
			val_type values;
		};

		inline int inherit_index(lua_State* L) noexcept
		{
#			if (LUA_VERSION_NUM >= 503)
			if (lua_rawgeti(L, -1, OBJ_FUNC) == LUA_TTABLE)
#			else
			lua_rawgeti(L, -1, OBJ_FUNC);
			if (lua_type(L, -1) == LUA_TTABLE)
#			endif
			{
				lua_pushvalue(L, 2);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) == LUA_TFUNCTION)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) == LUA_TFUNCTION)
#				endif
				{
					return 1;
				}
			}
			lua_settop(L, 3);
#			if (LUA_VERSION_NUM >= 503)
			if (lua_rawgeti(L, -1, OBJ_READER) == LUA_TTABLE)
#			else
			lua_rawgeti(L, -1, OBJ_READER);
			if (lua_type(L, -1) == LUA_TTABLE)
#			endif			
			{
				lua_pushvalue(L, 2);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) == LUA_TFUNCTION)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) == LUA_TFUNCTION)
#				endif
				{
					lua_pushvalue(L, 1);
					if (lua_pcall(L, 1, 1, 0))
					{
						return luaL_error(L, lua_tostring(L, -1));
					}
					else
					{
						return 1;
					}
				}
			}
			lua_settop(L, 3);
#			if (LUA_VERSION_NUM >= 503)
			if (lua_rawgeti(L, -1, OBJ_SUPER) == LUA_TTABLE)
#			else
			lua_rawgeti(L, -1, OBJ_SUPER);
			if (lua_type(L, -1) == LUA_TTABLE)
#			endif
			{
				size_t len = lua_rawlen(L, -1);
				for (size_t i(0); i < len; ++i)
				{
#					if (LUA_VERSION_NUM >= 503)
					if (lua_rawgeti(L, -1, i + 1) == LUA_TTABLE)
#					else
					lua_rawgeti(L, -1, int(i + 1));
					if (lua_type(L, -1) == LUA_TTABLE)
#					endif
					{
#						if (LUA_VERSION_NUM >= 503)
						if (lua_rawgeti(L, -1, OBJ_INDEX) == LUA_TFUNCTION)
#						else
						lua_rawgeti(L, -1, OBJ_INDEX);
						if (lua_type(L, -1) == LUA_TFUNCTION)
#						endif						
						{
							lua_pushvalue(L, 1);
							lua_pushvalue(L, 2);
							lua_pushvalue(L, 5);
							if (lua_pcall(L, 3, 1, 0))
							{
								return luaL_error(L, lua_tostring(L, -1));
							}
							
							else if (lua_type(L, -1) > LUA_TNIL)
							{
								return 1;
							}
						}
					}
					lua_settop(L, 4);
				}				
			}
			return 0;
		}

        inline int obj_index(lua_State* L) noexcept
		{
			if (lua_getmetatable(L, 1))
			{
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_INDEX) == LUA_TFUNCTION)
#				else
				lua_rawgeti(L, -1, OBJ_INDEX);
				if (lua_type(L, -1) == LUA_TFUNCTION)
#				endif
				{
					lua_pushvalue(L, 1);
					lua_pushvalue(L, 2);
					lua_pushvalue(L, 3);
					if (lua_pcall(L, 3, 1, 0))
					{
						return luaL_error(L, lua_tostring(L, -1));
					}
					else if (lua_type(L, -1) > LUA_TNIL)
					{
						return 1;
					}
					else
					{
						return luaL_error(L, "can not find readable symbol %s in an instance of %s",
							lua_tostring(L, 2), lua_tostring(L, lua_upvalueindex(1)));
					}
				}		
			}			
			return luaL_error(L, "wrong registered info in an instance of %s",
				lua_tostring(L, lua_upvalueindex(1)));
		}

		inline int inherit_newindex(lua_State* L) noexcept
		{
#			if (LUA_VERSION_NUM >= 503)
			if (lua_rawgeti(L, -1, OBJ_WRITER) == LUA_TTABLE)
#			else
			lua_rawgeti(L, -1, OBJ_WRITER);
			if (lua_type(L, -1) == LUA_TTABLE)
#			endif
			{
				lua_pushvalue(L, 2);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawget(L, -2) == LUA_TFUNCTION)
#				else
				lua_rawget(L, -2);
				if (lua_type(L, -1) == LUA_TFUNCTION)
#				endif				
				{
					lua_pushvalue(L, 1);
					lua_pushvalue(L, 3);
					if (lua_pcall(L, 2, 1, 0))
					{
						return luaL_error(L, lua_tostring(L, -1));
					}
					else
					{
						return 1;
					}
				}
			}
			lua_settop(L, 4);
#			if (LUA_VERSION_NUM >= 503)
			if (lua_rawgeti(L, -1, OBJ_SUPER) == LUA_TTABLE)
#			else
			lua_rawgeti(L, -1, OBJ_SUPER);
			if (lua_type(L, -1) == LUA_TTABLE)
#			endif
			{
				size_t len = lua_rawlen(L, -1);
				for (size_t i(0); i < len; ++i)
				{
#					if (LUA_VERSION_NUM >= 503)
					if (lua_rawgeti(L, -1, i + 1) == LUA_TTABLE)
#					else
					lua_rawgeti(L, -1, int(i + 1));
					if (lua_type(L, -1) == LUA_TTABLE)
#					endif					
					{
#						if (LUA_VERSION_NUM >= 503)
						if (lua_rawgeti(L, -1, OBJ_NEW_INDEX) == LUA_TFUNCTION)
#						else
						lua_rawgeti(L, -1, OBJ_NEW_INDEX);
						if (lua_type(L, -1) == LUA_TFUNCTION)
#						endif
						{
							lua_pushvalue(L, 1);
							lua_pushvalue(L, 2);
							lua_pushvalue(L, 3);
							lua_pushvalue(L, 6);
							if (lua_pcall(L, 4, 1, 0))
							{
								return luaL_error(L, lua_tostring(L, -1));
							}

							else if (lua_type(L, -1) > LUA_TNIL)
							{
								return 1;
							}
						}
					}
					lua_settop(L, 5);
				}
			}
			return 0;
		}

		inline int obj_newindex(lua_State* L) noexcept
		{
			if (lua_getmetatable(L, 1))
			{
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_NEW_INDEX) == LUA_TFUNCTION)
#				else
				lua_rawgeti(L, -1, OBJ_NEW_INDEX);
				if (lua_type(L, -1) == LUA_TFUNCTION)
#				endif
				{
					lua_pushvalue(L, 1);
					lua_pushvalue(L, 2);
					lua_pushvalue(L, 3);
					lua_pushvalue(L, 4);
					if (lua_pcall(L, 4, 1, 0))
					{
						return luaL_error(L, lua_tostring(L, -1));
					}
					else if (lua_type(L, -1) == LUA_TNUMBER)
					{
						auto res = lua_tointeger(L, -1);
						if (res == WRITER_SUCCEEDED)
						{
							return 0;
						}
						else
						{
							lua_getglobal(L, "tostring");
							lua_pushvalue(L, 3);
							lua_call(L, 1, 1);
							switch (res)
							{
							case WRITER_TYPE_FAILED:
								return luaL_error(L, "The type of %s is not suitable for symbol %s in an instance of %s.",
									lua_tostring(L, -1), lua_tostring(L, 2), lua_tostring(L, lua_upvalueindex(1)));
							case WRITER_VALUE_FAILED:
								return luaL_error(L, "%s is not a valid value for symbol %s in an instance of %s.",
									lua_tostring(L, -1), lua_tostring(L, 2), lua_tostring(L, lua_upvalueindex(1)));
							default:
								return luaL_error(L, "\"%s.%s = %s\" causing a unknown writer error.",
									lua_tostring(L, lua_upvalueindex(1)), lua_tostring(L, 2), lua_tostring(L, -1));
							}
						}
					}
					else if (lua_type(L, -1) == LUA_TNIL)
					{
						return luaL_error(L, "can not find writable symbol %s in an instance of %s.",
							lua_tostring(L, 2), lua_tostring(L, lua_upvalueindex(1)));
					}
					else
					{
						return luaL_error(L, "writer %s in an instance of %s is invalid.",
							lua_tostring(L, 2), lua_tostring(L, lua_upvalueindex(1)));
					}
				}
			}
			return luaL_error(L, "wrong registered info in an instance of %s",
				lua_tostring(L, lua_upvalueindex(1)));
		}

		template <class _Der, class... _Types>
		struct manual_member_reader : enrollment
		{
			typedef std::tuple<_Types...> holder;
			static_assert(type_traits<holder>::can_push, "holder types wrong.");

			manual_member_reader(const char* n, lua_CFunction f, _Types... pak) noexcept
				: name(n), func(f), upvalues(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_READER) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -1, OBJ_READER);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -3, OBJ_READER);
				}
				lua_pushstring(L, name);				
				lua_pushlightuserdata(L, &class_info<_Der>::info_data_map[get_main(L)]);
				LB_ASSERT_EQ(type_traits<holder>::push(L, upvalues),
					type_traits<holder>::stack_count);
				lua_pushcclosure(L, func, type_traits<holder>::stack_count + 1);
				lua_rawset(L, -3);
			}

			const char* name;
			lua_CFunction func;
			holder upvalues;
		};

		template <class _Der, class _Type>
		int value_member_reader(lua_State* L) noexcept
		{
			static_assert(type_traits<_Type>::can_push
				&& type_traits<_Type>::stack_count == 1, "wrong type for reader.");		
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				_Der* obj = (_Der*)get_adjusted_ptr((header*)lua_touserdata(L, 1),
					*(class_info_data*)lua_touserdata(L, lua_upvalueindex(1)));
				if (obj)
				{
					auto v = type_traits<_Type _Der::*>::get(L, lua_upvalueindex(2));
					return type_traits<_Type>::push(L, obj->*v);
				}
			}
			return 0;
		}

		template <class _Der, class _Type>
		int member_reader(lua_State* L) noexcept
		{
			static_assert(type_traits<_Type>::can_push
				&& type_traits<_Type>::stack_count == 1, "wrong type for reader.");
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				_Der* obj = (_Der*)get_adjusted_ptr((header*)lua_touserdata(L, 1),
					*(class_info_data*)lua_touserdata(L, lua_upvalueindex(1)));
				if (obj)
				{
					auto f = type_traits<_Type(_Der::*)()>::get(L, lua_upvalueindex(2));				
					return type_traits<_Type>::push(L, (obj->*f)());
				}
			}
			return 0;
		}

		template <class _Der, class... _Types>
		struct manual_member_writer : enrollment
		{
			typedef std::tuple<_Types...> holder;
			static_assert(type_traits<holder>::can_push, "holder types wrong.");

			manual_member_writer(const char* n, lua_CFunction f, _Types... pak) noexcept
				: name(n), func(f), upvalues(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, OBJ_WRITER) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -1, OBJ_WRITER);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -3, OBJ_WRITER);
				}
				lua_pushstring(L, name);
				lua_pushlightuserdata(L, &class_info<_Der>::info_data_map[get_main(L)]);
				LB_ASSERT_EQ(type_traits<holder>::push(L, upvalues),
					type_traits<holder>::stack_count);
				lua_pushcclosure(L, func, type_traits<holder>::stack_count + 1);
				lua_rawset(L, -3);
			}

			const char* name;
			lua_CFunction func;
			holder upvalues;
		};

		template <class _Der, class _Type>
		int value_member_writer(lua_State* L) noexcept
		{
			static_assert(type_traits<_Type>::can_get
				&& type_traits<_Type>::stack_count == 1, "wrong type for writer.");
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				_Der* obj = (_Der*)get_adjusted_ptr((header*)lua_touserdata(L, 1),
					*(class_info_data*)lua_touserdata(L, lua_upvalueindex(1)));
				if (obj)
				{
					if (type_traits<_Type>::test(L, -1))
					{
						auto v = type_traits<_Type _Der::*>::get(L, lua_upvalueindex(2));
						(obj->*v) = type_traits<_Type>::get(L, -1);
						lua_pushinteger(L, WRITER_SUCCEEDED);
					}
					else
					{
						lua_pushinteger(L, WRITER_TYPE_FAILED);
					}
					return 1;
				}
			}
			lua_pushinteger(L, WRITER_UNKNOWN_FIALED);
			return 1;
		}

		template <class _Der, class _Type>
		int member_writer(lua_State* L) noexcept
		{
			static_assert(type_traits<_Type>::can_get
				&& type_traits<_Type>::stack_count == 1, "wrong type for writer.");
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				_Der* obj = (_Der*)get_adjusted_ptr((header*)lua_touserdata(L, 1),
					*(class_info_data*)lua_touserdata(L, lua_upvalueindex(1)));
				if (obj)
				{
					if (type_traits<_Type>::test(L, -1))
					{
						auto f = type_traits<bool(_Der::*)(_Type)>::get(L, lua_upvalueindex(2));
						if ((obj->*f)(type_traits<_Type>::get(L, -1)))
						{
							lua_pushinteger(L, WRITER_SUCCEEDED);
						}
						else
						{
							lua_pushinteger(L, WRITER_VALUE_FAILED);
						}
					}
					else
					{
						lua_pushinteger(L, WRITER_TYPE_FAILED);
					}
					return 1;
				}
			}
			lua_pushinteger(L, WRITER_UNKNOWN_FIALED);
			return 1;
		}
	}

	template<class _Der, class... _Bases>
	struct base_finder;

	template<class _Der>
	struct base_finder<_Der>
	{
		static void find(env& e) noexcept
		{

		}

		static void fill(lua_State* L) noexcept
		{

		}
	};

	template<class _Der, class _This, class... _Rest>
	struct base_finder<_Der, _This, _Rest...>
	{
		static void find(env& e) noexcept
		{
			detail::class_info_data* info = &(detail::class_info<_Der>::info_data_map[e.L]);
			detail::class_info_data* super_info = &(detail::class_info<_This>::info_data_map[e.L]);
			ptrdiff_t diff = vtd::rtti::base::offset<_This, _Der>();
			info->base_map[super_info->type_id] = std::make_pair(diff, super_info);
			for (auto base : super_info->base_map)
			{
				info->base_map[base.first] = std::make_pair(
					base.second.first + diff, base.second.second);
			}
			super_info->sub_map[info->type_id] = std::make_pair(diff, super_info);
			base_finder<_Der, _Rest...>::find(e);
		}

		static void fill(lua_State* L) noexcept
		{
			auto len = lua_rawlen(L, -1);			
			detail::class_info_data* super_info = &(detail::class_info<_This>::info_data_map[get_main(L)]);
			LB_ASSERT(super_info->class_id);
			lua_rawgeti(L, LUA_REGISTRYINDEX, super_info->class_id);
			LB_ASSERT_EQ(lua_getmetatable(L, -1), 1);
			lua_rawgeti(L, -1, INDEX_CLASS);
			LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);			
			lua_rawseti(L, -4, int(len + 1));
			lua_pop(L, 2);			
			base_finder<_Der, _Rest...>::fill(L);
		}
	};
	
	template <class _Class, class _Func, class... _Types>
	struct class_func_def
	{
		static _Class& def(_Class& c, const char* name, _Func func, _Types... pak) noexcept
		{
			return c.def_func(name, func, pak...);
		}
	};

	template <class _Class, class _Val, class... _Types>
	struct class_val_def
	{
		
	};

	template <class _Class, class _Der, class _Type>
	struct class_val_helper_normal
	{
		static _Class& def(_Class& c, const char* name, _Type _Der::* val) noexcept
		{			
			return c.def_readonly(name, val).def_writeonly(name, val);
		}
	};

	template <class _Class, class _Der, class _Type>
	struct class_val_helper_const
	{
		static _Class& def(_Class& c, const char* name, _Type _Der::* val) noexcept
		{
			return c.def_readonly(name, val);
		}
	};

	template <class _Class, class _Der, class _Type>
	struct class_val_helper : std::conditional<std::is_const<_Type>::value,
		class_val_helper_const<_Class, _Der, _Type>,
		class_val_helper_normal<_Class, _Der, _Type>>::type
	{

	};

	template <class _Class, class _Der, class _Type>
	struct class_val_def<_Class, _Type _Der::*>
	{
		static _Class& def(_Class& c, const char* name, _Type _Der::* val) noexcept
		{
			return class_val_helper<_Class, _Der, _Type>::def(c, name, val);
		}
	};

	template <class _Class, class... _Types>
	struct class_member_def;

	template <class _Class, class _Flag, class... _Types>
	struct class_member_def<_Class, _Flag, _Types...> : std::conditional<
		std::is_member_function_pointer<_Flag>::value,
		class_func_def<_Class, _Flag, _Types...>,
		class_val_def<_Class, _Flag, _Types...>>::type
	{

	};

	template <class _Class, class _Constructor, class... _Types>
	struct class_constructor_def
	{
		static _Class& def(_Class& c, _Constructor, _Types... pak) noexcept
		{
			return c._Class::template def_constructor<
				decltype(_Constructor::template default_construct_func<typename _Class::_This>), _Types...>(
				&(_Constructor::template default_construct_func<typename _Class::_This>), pak...).
				_Class::template def_new<decltype(_Constructor::template default_new_func<typename _Class::_This>), _Types...>(
					&(_Constructor::template default_new_func<typename _Class::_This>), pak...);
		}
	};

	template <class _Class, class _Flag, class... _Types>
	struct class_def : std::conditional<
		std::is_same<_Flag, const char*>::value,
		class_member_def<_Class, _Types...>,
		class_constructor_def<_Class, _Flag, _Types...>>::type
	{

	};

	template<class _Der, class... _Bases>
	class class_ : public scope
	{
	public:
		typedef _Der _This;

		static_assert(std::is_class<_Der>::value && (!std::is_const<_Der>::value),
			"_Der need to be a class.");

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
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -1, INDEX_WRITER) == LUA_TTABLE)
#				else
				lua_rawgeti(L, -1, INDEX_WRITER);
				if (lua_type(L, -1) == LUA_TTABLE)
#				endif
				{
					lua_pushvalue(L, 2);
#					if (LUA_VERSION_NUM >= 503)
					if (lua_rawget(L, -2) == LUA_TFUNCTION)
#					else
					lua_rawget(L, -2);
					if (lua_type(L, -1) == LUA_TFUNCTION)
#					endif
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

		static int __gc(lua_State* L) noexcept
		{
			detail::header* data = (detail::header*)lua_touserdata(L, -1);
			if (data->type == USERDATA_CLASS)
			{
				switch (data->storage)
				{
				case STORAGE_LUA:
					(*(_Der*)(data + 1)).~_Der();
					break;
				case STORAGE_I_PTR:
					vtd::intrusive_obj<_Der>::dec((*(_Der**)(data + 1)));
					break;
				case STORAGE_U_PTR:
					(*(std::unique_ptr<_Der>*)(data + 1)).~unique_ptr<_Der>();
					break;
				case STORAGE_S_PTR:
					(*(std::shared_ptr<_Der>*)(data + 1)).~shared_ptr<_Der>();
					break;
				case STORAGE_W_PTR:
					(*(std::weak_ptr<_Der>*)(data + 1)).~weak_ptr<_Der>();
					break;
				default:
					break;
				}
			}
			return 0;
		}		

		struct enrollment : detail::enrollment
		{
			static detail::class_info_data* get_class_info(lua_State* L) noexcept
			{
				env& e = *get_env(L);
				detail::class_info_data* info = &(detail::class_info<_Der>::info_data_map[e.L]);
				if (!info->type_id)
				{
					info->type_id = int(e.class_map.size());
					e.class_map.push_back(info);
					info->base_map.clear();
					base_finder<_Der, _Bases...>::find(e);
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
#				if (LUA_VERSION_NUM >= 503)
				if (lua_rawgeti(L, -2, INDEX_CLASS) != LUA_TTABLE)
#				else
				lua_rawgeti(L, -2, INDEX_CLASS);
				if (lua_type(L, -1) != LUA_TTABLE)
#				endif				
				{
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1);
					lua_rawseti(L, -4, INDEX_CLASS);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &__gc);
					lua_rawset(L, -3);

					lua_pushstring(L, "__index");
					lua_rawgeti(L, -4, INDEX_SCOPE_NAME);					
					lua_pushcclosure(L, &detail::obj_index, 1);
					lua_rawset(L, -3);

					lua_pushstring(L, "__newindex");
					lua_rawgeti(L, -4, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &detail::obj_newindex, 1);
					lua_rawset(L, -3);

					lua_pushcclosure(L, &detail::inherit_index, 0);
					lua_rawseti(L, -2, OBJ_INDEX);

					lua_pushcclosure(L, &detail::inherit_newindex, 0);
					lua_rawseti(L, -2, OBJ_NEW_INDEX);
					
					if (sizeof...(_Bases))
					{
						lua_newtable(L);
						base_finder<_Der, _Bases...>::fill(L);
						LB_ASSERT(lua_rawlen(L, -1) == sizeof...(_Bases));
						lua_rawseti(L, -2, OBJ_SUPER);					
					}
				}
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

		template <class... _Types>
		class_& def(_Types... pak) noexcept
		{
			return class_def<class_, _Types...>::def(*this, pak...);
		}

		template <class _Func, class... _Types>
		class_& def_constructor(std::function<_Func> func, _Types... pak) noexcept
		{
			auto shell = create_func_shell<count_func_params((_Func*)nullptr) - (sizeof...(_Types))>(
				std::move(func));
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::construct_func<_Der, decltype(shell), _Types...>(shell, pak...)));
			return *this;
		}

		template <class _Func, class... _Types>
		class_& def_new(std::function<_Func> func, _Types... pak) noexcept
		{
			auto shell = create_func_shell<count_func_params((_Func*)nullptr) - (sizeof...(_Types))>(
				std::move(func));
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::new_func<_Der, decltype(shell), _Types...>(shell, pak...)));
			return *this;
		}

		template <class _Func, class... _Types>
		class_& def_func(const char* name, _Func func, _Types... pak) noexcept
		{
			static_assert(std::is_member_function_pointer<_Func>::value,
				"func has to be a member function.");
			auto shell = create_member_func_shell<count_func_params((_Func)nullptr) - (sizeof...(_Types))>(func);
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::member_func<decltype(shell), _Types...>(name, shell, pak...)));
			return *this;
		}
		
		template <class... _Types>
		class_& def_manual(const char* name, lua_CFunction func, _Types... pak) noexcept
		{
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::manual_member_func<_Types...>(name, func, pak...)));
			return *this;
		}

		template <class... _Types>
		class_& def_manual_reader(const char* name, lua_CFunction func, _Types... pak) noexcept
		{
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::manual_member_reader<_Der, _Types...>(name, func, pak...)));
			return *this;
		}

		template <class _Type>
		class_& def_readonly(const char* name, _Type _Der::* val) noexcept
		{
			return def_manual_reader(name, &detail::value_member_reader<_Der, _Type>, val);
		}

		template <class _Type>
		class_& def_reader(const char* name, _Type(_Der::*func)()) noexcept
		{
			return def_manual_reader(name, &detail::member_reader<_Der, _Type>, func);
		}

		template <class... _Types>
		class_& def_manual_writer(const char* name, lua_CFunction func, _Types... pak) noexcept
		{
			((enrollment*)chain)->member_scope.operator,
				(scope(new detail::manual_member_writer<_Der, _Types...>(name, func, pak...)));
			return *this;
		}

		template <class _Type>
		class_& def_writeonly(const char* name, _Type _Der::* val) noexcept
		{
			return def_manual_writer(name, &detail::value_member_writer<_Der, _Type>, val);
		}

		template <class _Type>
		class_& def_writer(const char* name, bool(_Der::*func)(_Type)) noexcept
		{
			return def_manual_writer(name, &detail::member_writer<_Der, _Type>, func);
		}

	};
}
