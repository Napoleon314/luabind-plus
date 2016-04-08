////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   invoke.h
//  Created:     2016/04/07 by Albert D Yang
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
	template <int idx, class... _Types>
	struct params_trimmer;
	
	template <int idx>
	struct params_trimmer<idx>
	{
		typedef std::tuple<> type;
	};

	template <int idx, class _This, class... _Rest>
	struct params_trimmer<idx, _This, _Rest...>
	{
		typedef typename std::conditional < (idx > 0),
			typename params_trimmer<idx - 1, _Rest...>::type,
			typename std::tuple < _This, _Rest... >> ::type type;
	};
	
	template <int idx, class... _Rest>
	struct params_finder;

	template <int idx>
	struct params_finder<idx>
	{
		typedef void type;
	};

	template <int idx, class _This, class... _Rest>
	struct params_finder<idx, _This, _Rest...>
	{
		typedef typename std::conditional < (idx > 0),
			typename params_finder<idx - 1, _Rest...>::type,
			typename _This>::type type;
	};

	template <class _Ret, class... _Types>
	inline constexpr int count_func_params(_Ret(*)(_Types...)) noexcept
	{
		return sizeof...(_Types);
	}

	template <int stack_base, int param_idx, int idx, class... _Types>
	struct func_tester;

	template <int stack_base, int param_idx, int idx>
	struct func_tester<stack_base, param_idx, idx>
	{
		static bool test(lua_State* L, int top) noexcept
		{
			return stack_base == top;
		}
	};

	template <int stack_base, int param_idx, int idx, class _This, class... _Rest>
	struct func_tester<stack_base, param_idx, idx, _This, _Rest...>
	{
		static bool test(lua_State* L, int top) noexcept
		{
			if (top > stack_base)
			{
				constexpr int next_stack_cap = stack_base + type_traits<_This>::stack_count;
				if (next_stack_cap <= top)
				{
					return type_traits<_This>::test(L, stack_base + 1)
						&& func_tester<next_stack_cap, param_idx + 1, idx, _Rest...>::test(L, top);
				}
				else
				{
					return false;
				}
			}
			else if(top == stack_base && type_traits<_This>::stack_count == 0)
			{
				return func_tester<stack_base, param_idx + 1, idx, _Rest...>::test(L, top);
			}
			else
			{
				return param_idx >= idx;
			}
		}
	};

	template <class _Shell, class... _Types>
	struct func_params_maker
	{
		static typename _Shell::ret_type invoke(typename _Shell::func_type& f,
			typename _Shell::val_type& v, lua_State* L,
			int top, int base, _Types... pak) noexcept
		{
			auto p = _Shell::get<sizeof...(_Types)>(v, L, top, base);
			base += type_traits<decltype(p)>::stack_count;
			return func_invoker<_Shell, _Types..., decltype(p)>::invoke(
				f, v, L, top, base, pak..., p);
		}
	};

	template <class _Shell, class... _Types>
	struct func_caller
	{
		static typename _Shell::ret_type invoke(typename _Shell::func_type& f,
			typename _Shell::val_type& v, lua_State* L,
			int top, int base, _Types... pak) noexcept
		{
			return f(pak...);
		}
	};

	template <class _Shell, class... _Types>
	struct func_invoker : std::conditional<
		sizeof...(_Types) == _Shell::params_count,
		func_caller<_Shell, _Types...>,
		func_params_maker<_Shell, _Types...>	
	>::type
	{

	};
}
