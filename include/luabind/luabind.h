////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      luabind_plus
//  File name:   luabind.h
//  Created:     2016/04/01 by Albert D Yang
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

#ifndef LB_ASSERT
#define LB_ASSERT()
#endif

#ifndef LB_LOG_D
#define LB_LOG_D()
#endif

#ifndef LB_LOG_I
#define LB_LOG_I()
#endif

#ifndef LB_LOG_W
#define LB_LOG_W()
#endif

#ifndef LB_LOG_E
#define LB_LOG_E()
#endif

#ifndef LB_BUF_SIZE
#define LB_BUF_SIZE (1024)
#endif

#ifndef NDEBUG
#define LB_ASSERT_EQ(e,v) LB_ASSERT(e == v)
#else
#define LB_ASSERT_EQ(e,v) e
#endif

#include "detail/utility.h"
#include "detail/type_traits.h"
#include "detail/environment.h"
#include "detail/invoke.h"
#include "detail/function.h"
#include "detail/object.h"
#include "detail/scope.h"
#include "detail/class.h"

namespace luabind
{

}

#undef LB_ASSERT_EQ

#ifdef LB_ASSERT
#undef LB_ASSERT
#endif

#ifdef LB_LOG_D
#undef LB_LOG_D
#endif

#ifdef LB_LOG_I
#undef LB_LOG_I
#endif

#ifdef LB_LOG_W
#undef LB_LOG_W
#endif

#ifdef LB_LOG_E
#undef LB_LOG_E
#endif

#ifdef LB_BUF_SIZE
#undef LB_BUF_SIZE
#endif
