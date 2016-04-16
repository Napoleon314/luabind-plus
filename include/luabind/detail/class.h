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

	template <class... _Types>
	struct constructor
	{
		typedef void func_type(void*, _Types...);

		template <class _Der>
		static void default_constructor(void* m, _Types... pak) noexcept
		{
			new(m) _Der(pak...);
		}
	}; 

	namespace detail
	{
		struct header
		{
			short type;
			short storage;
			int type_id;
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
				if (lua_rawgeti(L, -3, INDEX_CONSTRUCTOR) != LUA_TUSERDATA)
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
					lua_pushcclosure(L, &func_holder::construct_entry, 3);
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
	}

	template <class _Ty>
	struct object_traits
	{
		typedef typename std::remove_cv<_Ty>::type type;
		static_assert(std::is_class<type>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			if (lua_type(L, idx) == LUA_TUSERDATA)
			{
				detail::header* data = (detail::header*)lua_touserdata(L, idx);
				if (data->type == USERDATA_CLASS && data->storage == STORAGE_LUA)
				{
					auto it = detail::class_info<type>::info_data_map.find(get_main(L));
					if (it != detail::class_info<type>::info_data_map.end())
					{
						if (data->type_id == it->second.type_id)
						{
							return true;
						}
					}
				}
			}			
			return false;
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			detail::header* data = (detail::header*)lua_touserdata(L, idx);
			return *(_Ty*)(data + 1);
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			detail::header* data = (detail::header*)lua_newuserdata(L, sizeof(_Ty) + sizeof(detail::header));
			data->type = USERDATA_CLASS;
			data->storage = STORAGE_LUA;			
			data->type_id = detail::class_info<_Ty>::info_data_map[get_main(L)].type_id;
			new(data + 1) _Ty(val);
			return 1;
		}

		static _Ty make_default() noexcept
		{
			return _Ty();
		}
	};

	template <class _Ty>
	struct object_traits<_Ty&>
	{
		typedef typename std::remove_cv<_Ty>::type type;
		static_assert(std::is_class<type>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			if (lua_type(L, idx) == LUA_TUSERDATA)
			{
				detail::header* data = (detail::header*)lua_touserdata(L, idx);
				if (data->type == USERDATA_CLASS)
				{
					auto it = detail::class_info<type>::info_data_map.find(get_main(L));
					if (it != detail::class_info<type>::info_data_map.end())
					{
						if (data->type_id == it->second.type_id)
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		static _Ty& get(lua_State* L, int idx) noexcept
		{
			detail::header* data = (detail::header*)lua_touserdata(L, idx);
			switch (data->storage)
			{
			case STORAGE_LUA:
				return *(_Ty*)(data + 1);
			case STORAGE_I_PTR:
				return **(_Ty**)(data + 1);
			case STORAGE_U_PTR:
				return **(std::unique_ptr<_Ty>*)(data + 1);
			case STORAGE_S_PTR:
				return **(std::shared_ptr<_Ty>*)(data + 1);
			case STORAGE_W_PTR:
				return *(*(std::weak_ptr<_Ty>*)(data + 1)).lock();
			default:
				break;
			}
			return make_default();
		}

		static int push(lua_State* L, _Ty& val) noexcept
		{
			detail::header* data = (detail::header*)lua_newuserdata(L, sizeof(_Ty) + sizeof(detail::header));
			data->type = USERDATA_CLASS;
			data->storage = STORAGE_LUA;
			data->type_id = detail::class_info<_Ty>::info_data_map[get_main(L)].type_id;
			new(data + 1) _Ty(val);
			return 1;
		}

		static _Ty& make_default() noexcept
		{
			return *(_Ty*)nullptr;
		}
	};

	template <class _Ty>
	struct object_traits<_Ty&&>
	{
		typedef typename std::remove_cv<_Ty>::type type;
		static_assert(std::is_class<type>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return false;
		}

		static _Ty&& get(lua_State* L, int idx) noexcept
		{			
			return make_default();
		}

		static int push(lua_State* L, _Ty& val) noexcept
		{
			detail::header* data = (detail::header*)lua_newuserdata(L, sizeof(_Ty) + sizeof(detail::header));
			data->type = USERDATA_CLASS;
			data->storage = STORAGE_LUA;
			data->type_id = detail::class_info<_Ty>::info_data_map[get_main(L)].type_id;
			new(data + 1) _Ty(val);
			return 1;
		}

		static _Ty&& make_default() noexcept
		{
			return std::move(_Ty());
		}
	};

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
		static_assert(std::is_class<_Der>::value, "_Der need to be a class.");

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

		static int __gc(lua_State* L) noexcept
		{
			detail::header* data = (detail::header*)lua_touserdata(L, -1);
			if (data->type == USERDATA_CLASS)
			{
				switch (data->storage)
				{
				case STORAGE_LUA:
					((_Der*)(data + 1))->~_Der();
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
					lua_pushvalue(L, -1);
					lua_rawseti(L, -4, INDEX_CLASS);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &__gc);
					lua_rawset(L, -3);
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

		template <class _Constructor, class... _Types>
		class_& def(_Constructor, _Types... pak) noexcept
		{
			return def_constructor<typename _Constructor::func_type, _Types...>(
				&(_Constructor::template default_constructor<_Der>), pak...);
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
		
	};
}
