////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016-2026
// -------------------------------------------------------------------------
//  Module:      luabind_plus_test
//  File name:   main.cpp
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

#include <vtd/rtti.h>
#include <luabind/luabind.h>
extern "C"
{
#	include "lua/lua.h"
#	include "lua/lualib.h"
#	include "lua/lauxlib.h"
}

class A
{
public:
	vtd_rtti_decl(A);



	virtual ~A() {}

	int a = 5;

};

vtd_rtti_impl(A);

class B
{
public:
	vtd_rtti_decl(B);

	virtual ~B() {}

	int b = 8;

};

vtd_rtti_impl(B);

class C : public A, public B
{
public:
	vtd_rtti_decl(C, A, B);

	virtual ~C() {}

	int c = 11;

};

vtd_rtti_impl(C, A, B);

int main()
{
	C* pkC = new C;
	A* pkA = static_cast<A*>(pkC);
	B* pkB = static_cast<B*>(pkC);


	bool b = vtd_is_kind_of(B, pkA);
	b = vtd_is_exact_kind_of(B, pkA);


	/*VeSizeT stOffset = VeStd::base_offset<B,C>();

	VeSizeT stTemp = B::ms_RTTI.GetPathFrom(&C::ms_kRTTI);*/

	A* pkA2 = vtd_dynamic_cast(A, pkB);
	B* pkB2 = vtd_dynamic_cast(B, pkB);
	C* pkC2 = vtd_dynamic_cast(C, pkC);
	C* pkC3 = vtd_dynamic_cast(C, pkB);

	printf("%d,%d,%d,%d,%d", pkA2->a, pkB2->b, pkC2->c, pkC3->c, b);

	return 0;
}
