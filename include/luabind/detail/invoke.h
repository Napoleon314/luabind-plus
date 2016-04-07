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
	/*template <class... _Types>
	struct params_checker;

	template <>
	struct params_checker<>
	{
		template <class... _Targets>
		struct targets
		{
			static constexpr bool is_matched = !(sizeof...(_Targets));
		};
	};

	template <class _This, class... _Rest>
	struct params_checker<_This, _Rest...>
	{
		template <class... _Types>
		struct targets;

		template <>
		struct targets<int>
		{
			static constexpr bool is_matched = false;
		};

		template <class _This1, class... _Rest1>
		struct targets<_This1, _Rest1...>
		{
			static constexpr bool is_matched = (sizeof...(_Rest)) > (sizeof...(_Rest1))
				? params_checker<_Rest...>::targets<_This1, _Rest1...>::is_matched
				: ((sizeof...(_Rest)) == (sizeof...(_Rest1))
					? (std::is_same<_This, _This1>::value
						&& params_checker<_Rest...>::targets<_Rest1...>::is_matched)
					: false);	
		};
	};*/

	/*template <class _This1, class... _Rest1, int idx, class _This2, class... _Rest2>
	struct params_checker
	{
		constexpr bool is_matched = idx ? 1 : 0;
	};*/
}
