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

namespace luabind
{
	struct scope;

	enum SocpeType
	{
		SCOPE_NAMESPACE,
		SCOPE_ENUM,
		SCOPE_CLASS,
		SCOPE_MAX
	};

	namespace detail
	{
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
            friend struct luabind::scope;
			enrollment* next = nullptr;
		};

		template <class _Type>
		struct const_value : enrollment
		{
			static_assert(type_traits<_Type>::stack_count == 1
				&& type_traits<_Type>::can_push, "type of const value wrong.");

			const_value(const char* n, _Type v) noexcept
				: name(n), value(v) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				lua_pushstring(L, name);
				LB_ASSERT_EQ(type_traits<_Type>::push(L, value), 1);
				lua_rawset(L, -3);
			}

			const char* name;
			_Type value;
		};

		template <class... _Types>
		struct manual_func : enrollment
		{
			typedef std::tuple<_Types...> holder;

			manual_func(const char* n, lua_CFunction f, _Types... pak) noexcept
				: name(n), func(f), upvalues(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);
				lua_pushstring(L, name);
				if (type_traits<holder>::can_push)
				{
					if (type_traits<holder>::push(L, upvalues)
						== type_traits<holder>::stack_count)
					{
						lua_pushcclosure(L, func, type_traits<holder>::stack_count);
						lua_rawset(L, -3);
					}
				}
			}

			const char* name;
			lua_CFunction func;
			holder upvalues;
		};

		template <class _Shell, class... _Types>
		struct cpp_func : enrollment
		{
			typedef typename _Shell::func_type func_type;
			typedef typename _Shell::val_type val_type;

			cpp_func(const char* n, _Shell& f, _Types... pak) noexcept
				: name(n), func(std::move(f.func)), values(pak...) {}

			virtual void enroll(lua_State* L) const noexcept
			{
				LUABIND_HOLD_STACK(L);				
				if (lua_rawgeti(L, -2, INDEX_FUNC) != LUA_TTABLE)
				{
					lua_pop(L, 1);
					lua_newtable(L);					
					lua_pushvalue(L, -1);					
					lua_rawseti(L, -4, INDEX_FUNC);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
				lua_pushstring(L, name);
				if (lua_rawget(L, -2) != LUA_TUSERDATA)
				{
					lua_pop(L, 1);
					void* data = lua_newuserdata(L, sizeof(func_holder*));
					*(func_holder**)data = new func_holder_impl<_Shell>(func, values);
					lua_newtable(L);
					lua_pushstring(L, "__gc");
					lua_pushcfunction(L, &func_holder::__gc);
					lua_rawset(L, -3);
					lua_setmetatable(L, -2);
					lua_pushstring(L, name);
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
					lua_pushstring(L, name);
					lua_pushcclosure(L, &func_holder::entry, 2);
					lua_pushstring(L, name);
					lua_pushvalue(L, -2);
					lua_rawset(L, -5);
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
							h->next = new func_holder_impl<_Shell>(func, values);
							break;
						}						
					}					
				}
			}

			const char* name;
			func_type func;
			val_type values;
		};
	}

	struct scope
	{
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
			detail::enrollment** cur = &chain;
			while (*cur)
			{
				cur = &((*cur)->next);
			}
			*cur = s.chain;
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

	class namespace_ : public scope
	{
	public:
		static int __tostring(lua_State* L) noexcept
		{
			char buf[LB_BUF_SIZE];
			const char* name = lua_tostring(L, lua_upvalueindex(1));
			if (*name)
			{
				sprintf(buf, "c++ namespace[%s]", name);
			}
			else
			{
				sprintf(buf, "c++ namespace[_G]");
			}
			lua_pushstring(L, buf);
			return 1;
		}

		static int __newindex(lua_State* L) noexcept
		{
			lua_pushvalue(L, lua_upvalueindex(1));
			lua_pushvalue(L, -3);
			if (lua_gettable(L, -5) > 1)
			{				
				lua_getglobal(L, "tostring");
				lua_pushvalue(L, 2);
				lua_call(L, 1, 1);
				const char* s = lua_tostring(L, -1);
				const char* scope_name = lua_tostring(L, lua_upvalueindex(2));
				if (*scope_name)
				{
					return luaL_error(L, "new index \"%s.%s\" causing a luabind name conflict.", scope_name, s);
				}
				else
				{
					return luaL_error(L, "new index \"%s\" causing a luabind name conflict.", s);
				}
			}
			else
			{
				lua_settop(L, 3);
				lua_rawset(L, -3);
			}
			return 0;
		}

		struct enrollment : detail::enrollment
		{
			enrollment(const char* n) noexcept
				: name(n)
			{

			}

			virtual void enroll(lua_State* L) const noexcept
			{
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
					lua_pushinteger(L, SCOPE_NAMESPACE);
					lua_rawseti(L, -2, INDEX_SCOPE);
					lua_pushstring(L, full_name);
					lua_rawseti(L, -2, INDEX_SCOPE_NAME);
					lua_pushstring(L, "__tostring");
					lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &namespace_::__tostring, 1);
					lua_rawset(L, -3);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -4, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &namespace_::__newindex, 2);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}				
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifndef NDEBUG
				lua_rawgeti(L, -2, INDEX_SCOPE);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == SCOPE_NAMESPACE);
				lua_pop(L, 1);
#				endif
				inner_scope.enroll(L);
				lua_pop(L, 3);
			}

			const char* name;
			scope inner_scope;
		};


		explicit namespace_(const char* name) noexcept
			: scope(new enrollment(name))
		{

		}

		namespace_& operator [] (scope s) noexcept
		{
			((enrollment*)chain)->inner_scope.operator,(s);
			return *this;
		}
	};

	class module_
	{
	public:
		module_(lua_State* L, const char* n) noexcept
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
					lua_pushinteger(L, SCOPE_NAMESPACE);
					lua_rawseti(L, -2, INDEX_SCOPE);
					lua_pushstring(L, "");
					lua_rawseti(L, -2, INDEX_SCOPE_NAME);
					lua_pushstring(L, "__tostring");
					lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &namespace_::__tostring, 1);
					lua_rawset(L, -3);
					lua_newtable(L);
					lua_pushstring(L, "__newindex");
					lua_pushvalue(L, -2);
					lua_rawgeti(L, -4, INDEX_SCOPE_NAME);
					lua_pushcclosure(L, &namespace_::__newindex, 2);
					lua_rawset(L, -4);
					lua_pushstring(L, "__index");
					lua_pushvalue(L, -2);
					lua_rawset(L, -4);
				}
				LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#				ifndef NDEBUG
				lua_rawgeti(L, -2, INDEX_SCOPE);
				LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
					&& lua_tointeger(L, -1) == SCOPE_NAMESPACE);
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
						lua_pushinteger(L, SCOPE_NAMESPACE);
						lua_rawseti(L, -2, INDEX_SCOPE);
						lua_pushstring(L, name);
						lua_rawseti(L, -2, INDEX_SCOPE_NAME);
						lua_pushstring(L, "__tostring");
						lua_rawgeti(L, -2, INDEX_SCOPE_NAME);
						lua_pushcclosure(L, &namespace_::__tostring, 1);
						lua_rawset(L, -3);
						lua_newtable(L);
						lua_pushstring(L, "__newindex");
						lua_pushvalue(L, -2);
						lua_rawgeti(L, -4, INDEX_SCOPE_NAME);
						lua_pushcclosure(L, &namespace_::__newindex, 2);
						lua_rawset(L, -4);
						lua_pushstring(L, "__index");
						lua_pushvalue(L, -2);
						lua_rawset(L, -4);
					}
					LB_ASSERT(lua_type(L, -1) == LUA_TTABLE);
#					ifndef NDEBUG
					lua_rawgeti(L, -2, INDEX_SCOPE);
					LB_ASSERT(lua_type(L, -1) == LUA_TNUMBER
						&& lua_tointeger(L, -1) == SCOPE_NAMESPACE);
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

	inline module_ module(lua_State* L, char const* name = nullptr) noexcept
	{
		return module_(L, name);
	}

	template <class... _Types>
	scope def_manual(const char* name, lua_CFunction func, _Types... pak) noexcept
	{
		return scope(new detail::manual_func<_Types...>(name, func, pak...));
	}

	template <class _Func, class... _Types>
	scope def(const char* name, std::function<_Func> func, _Types... pak) noexcept
	{
		auto shell = create_func_shell<count_func_params((_Func*)nullptr)-(sizeof...(_Types))>(
			std::move(func));
		return scope(new detail::cpp_func<decltype(shell), _Types...>(name, shell, pak...));
	}

	template <class _Func, class... _Types>
	scope def(const char* name, _Func* func, _Types... pak) noexcept
	{
		static_assert(std::is_function<_Func>::value, "_Func has to be a function.");
		return def(name, std::function<_Func>(func), pak...);
	}

	template <class _Type>
	scope def_const(const char* name, _Type val) noexcept
	{
		return scope(new detail::const_value<_Type>(name, val));
	}
}
