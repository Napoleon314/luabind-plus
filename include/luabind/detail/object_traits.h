////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   object_traits.h
//  Created:     2016/04/16 by Albert D Yang
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
	namespace detail
	{
		template <class _Ty>
		bool test_obj(lua_State* L, int idx) noexcept
		{
			if (lua_type(L, idx) == LUA_TUSERDATA)
			{
				detail::header* info = (detail::header*)lua_touserdata(L, idx);
				if (info->type == USERDATA_CLASS)
				{
					auto it = detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.find(get_main(L));
					if (it != detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.end())
					{
						if (info->type_id == it->second.type_id)
						{
							switch (info->storage)
							{
							case STORAGE_U_PTR:
								return (*(std::unique_ptr<_Ty>*)(info + 1)) != nullptr;
							case STORAGE_W_PTR:
								return !((*(std::weak_ptr<_Ty>*)(info + 1)).expired());
							default:
								return true;
							}
						}
					}
				}
			}
			return false;
		}

		template <class _Ty, storage_type s>
		bool test_obj(lua_State* L, int idx) noexcept
		{
			if (lua_type(L, idx) == LUA_TUSERDATA)
			{
				detail::header* info = (detail::header*)lua_touserdata(L, idx);
				if (info->type == USERDATA_CLASS && info->storage == s)
				{
					auto it = detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.find(get_main(L));
					if (it != detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.end())
					{
						if (info->type_id == it->second.type_id)
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		template <class _Ty>
		bool test_ptr(lua_State* L, int idx) noexcept
		{
			if (lua_type(L, idx) == LUA_TUSERDATA)
			{
				detail::header* info = (detail::header*)lua_touserdata(L, idx);
				if (info->type == USERDATA_CLASS
					&& (info->storage == STORAGE_S_PTR || info->storage == STORAGE_W_PTR))
				{
					auto it = detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.find(get_main(L));
					if (it != detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map.end())
					{
						if (info->type_id == it->second.type_id)
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		template <class _Ty>
		_Ty& get_obj(lua_State* L, int idx) noexcept
		{
			detail::header* info = (detail::header*)lua_touserdata(L, idx);
			switch (info->storage)
			{
			case STORAGE_LUA:
				return *(_Ty*)(info + 1);
			case STORAGE_I_PTR:
				return **(_Ty**)(info + 1);
			case STORAGE_U_PTR:
				return **(std::unique_ptr<_Ty>*)(info + 1);
			case STORAGE_S_PTR:
				return **(std::shared_ptr<_Ty>*)(info + 1);
			case STORAGE_W_PTR:
				return *(*(std::weak_ptr<_Ty>*)(info + 1)).lock();
			default:
				break;
			}
			return *(_Ty*)nullptr;
		}		

		template <class _Ty, storage_type s>
		userdata_obj<_Ty, s>* push_obj(lua_State* L) noexcept
		{
			auto& info = detail::class_info<typename std::remove_cv<_Ty>::type>::info_data_map[get_main(L)];
			LB_ASSERT(info.class_id);
			auto data = (userdata_obj<_Ty, s>*)lua_newuserdata(L, sizeof(userdata_obj<_Ty, s>));
			data->info.type = USERDATA_CLASS;
			data->info.storage = s;
			data->info.type_id = info.type_id;
			LB_ASSERT_EQ(lua_rawgeti(L, LUA_REGISTRYINDEX, info.class_id), LUA_TTABLE);
			LB_ASSERT_EQ(lua_getmetatable(L, -1), 1);
			LB_ASSERT_EQ(lua_rawgeti(L, -1, INDEX_CLASS), LUA_TTABLE);
			lua_setmetatable(L, -4);
			lua_pop(L, 2);
			return data;
		}
	}

	template <class _Ty>
	struct object_traits
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_LUA>(L, idx);
		}

		static _Ty get(lua_State* L, int idx) noexcept
		{
			return detail::get_obj<_Ty>(L, idx);
		}

		static int push(lua_State* L, _Ty val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_LUA>(L);
			new (&obj->data) _Ty(val);
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
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty>(L, idx);
		}

		static _Ty& get(lua_State* L, int idx) noexcept
		{
			return detail::get_obj<_Ty>(L, idx);
		}

		static int push(lua_State* L, _Ty& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_LUA>(L);
			new (&obj->data) _Ty(val);
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
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, _Ty&& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_LUA>(L);
			new (&obj->data) _Ty(val);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<_Ty*>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_I_PTR>(L, idx);
		}

		static _Ty* get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_I_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_I_PTR);
			return obj->data;
		}

		static int push(lua_State* L, _Ty* val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);			
			obj->data = val;
			vtd::intrusive_obj<type>::inc(obj->data);
			return 1;
		}

		static _Ty* make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct object_traits<std::unique_ptr<_Ty>>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_U_PTR>(L, idx);
		}

		static std::unique_ptr<_Ty> get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_U_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_U_PTR);
			return std::move(obj->data);
		}

		static int push(lua_State* L, std::unique_ptr<_Ty> val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_U_PTR>(L);
			new (&obj->data) std::unique_ptr<_Ty>(std::move(val));
			return 1;
		}

		static std::unique_ptr<_Ty> make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct object_traits<std::unique_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, std::unique_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_U_PTR>(L);
			new (&obj->data) std::unique_ptr<_Ty>(std::move(val));
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<const std::unique_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = false;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_U_PTR>(L, idx);
		}

		static const std::unique_ptr<_Ty>& get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_U_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_U_PTR);
			return obj->data;
		}
	};


	template <class _Ty>
	struct object_traits<std::unique_ptr<_Ty>&&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, std::unique_ptr<_Ty>&& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_U_PTR>(L);
			new (&obj->data) std::unique_ptr<_Ty>(std::move(val));
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<std::shared_ptr<_Ty>>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_ptr<_Ty>(L, idx);
		}

		static std::shared_ptr<_Ty> get(lua_State* L, int idx) noexcept
		{
			auto info = (detail::header*)lua_touserdata(L, idx);
			if (info->storage == STORAGE_S_PTR)
			{
				return ((detail::userdata_obj<_Ty, STORAGE_S_PTR>*)info)->data;
			}
			else
			{
				LB_ASSERT(info->storage == STORAGE_W_PTR);
				return ((detail::userdata_obj<_Ty, STORAGE_W_PTR>*)info)->data.lock();
			}
		}

		static int push(lua_State* L, std::shared_ptr<_Ty> val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_S_PTR>(L);
			new (&obj->data) std::shared_ptr<_Ty>(val);
			return 1;
		}

		static std::shared_ptr<_Ty> make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct object_traits<std::shared_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;		

		static int push(lua_State* L, std::shared_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_S_PTR>(L);
			new (&obj->data) std::shared_ptr<_Ty>(val);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<const std::shared_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_S_PTR>(L, idx);
		}

		static const std::shared_ptr<_Ty>& get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_S_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_S_PTR);
			return obj->data;
		}

		static int push(lua_State* L, const std::shared_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_S_PTR>(L);
			new (&obj->data) std::shared_ptr<_Ty>(val);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<std::shared_ptr<_Ty>&&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, std::shared_ptr<_Ty>&& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_S_PTR>(L);
			new (&obj->data) std::shared_ptr<_Ty>(std::move(val));
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<std::weak_ptr<_Ty>>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_ptr<_Ty>(L, idx);
		}

		static std::weak_ptr<_Ty> get(lua_State* L, int idx) noexcept
		{
			auto info = (detail::header*)lua_touserdata(L, idx);
			if (info->storage == STORAGE_S_PTR)
			{
				return std::weak_ptr<_Ty>(((detail::userdata_obj<_Ty, STORAGE_S_PTR>*)info)->data);
			}
			else
			{
				LB_ASSERT(info->storage == STORAGE_W_PTR);
				return ((detail::userdata_obj<_Ty, STORAGE_W_PTR>*)info)->data;
			}
		}

		static int push(lua_State* L, std::weak_ptr<_Ty> val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_W_PTR>(L);
			new (&obj->data) std::weak_ptr<_Ty>(val);
			return 1;
		}

		static std::weak_ptr<_Ty> make_default() noexcept
		{
			return std::weak_ptr<_Ty>();
		}
	};

	template <class _Ty>
	struct object_traits<std::weak_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, std::weak_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_W_PTR>(L);
			new (&obj->data) std::weak_ptr<_Ty>(val);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<const std::weak_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_W_PTR>(L, idx);
		}

		static const std::weak_ptr<_Ty>& get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_W_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_W_PTR);
			return obj->data;
		}

		static int push(lua_State* L, const std::weak_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_W_PTR>(L);
			new (&obj->data) std::weak_ptr<_Ty>(val);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<std::weak_ptr<_Ty>&&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, std::weak_ptr<_Ty>&& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_W_PTR>(L);
			new (&obj->data) std::weak_ptr<_Ty>(std::move(val));
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<vtd::intrusive_ptr<_Ty>>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_I_PTR>(L, idx);
		}

		static vtd::intrusive_ptr<_Ty> get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_I_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_I_PTR);
			return obj->data;
		}

		static int push(lua_State* L, vtd::intrusive_ptr<_Ty> val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);			
			obj->data = val;
			vtd::intrusive_obj<type>::inc(obj->data);
			return 1;
		}

		static vtd::intrusive_ptr<_Ty> make_default() noexcept
		{
			return nullptr;
		}
	};

	template <class _Ty>
	struct object_traits<vtd::intrusive_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, vtd::intrusive_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
			obj->data = val;
			vtd::intrusive_obj<type>::inc(obj->data);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<const vtd::intrusive_ptr<_Ty>&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = true;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static bool test(lua_State* L, int idx) noexcept
		{
			return detail::test_obj<_Ty, STORAGE_I_PTR>(L, idx);
		}

		static const vtd::intrusive_ptr<_Ty>& get(lua_State* L, int idx) noexcept
		{
			auto obj = (detail::userdata_obj<_Ty, STORAGE_I_PTR>*)lua_touserdata(L, idx);
			LB_ASSERT(obj->info.storage == STORAGE_I_PTR);
			return *(vtd::intrusive_ptr<_Ty>*)&(obj->data);
		}

		static int push(lua_State* L, const vtd::intrusive_ptr<_Ty>& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
			obj->data = val;
			vtd::intrusive_obj<type>::inc(obj->data);
			return 1;
		}
	};

	template <class _Ty>
	struct object_traits<vtd::intrusive_ptr<_Ty>&&>
	{
		static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

		static constexpr bool can_get = false;

		static constexpr bool can_push = true;

		static constexpr int stack_count = 1;

		static int push(lua_State* L, vtd::intrusive_ptr<_Ty>&& val) noexcept
		{
			auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
			obj->data = val;
			vtd::intrusive_obj<type>::inc(obj->data);
			return 1;
		}
	};
}
