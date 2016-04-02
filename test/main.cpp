////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
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

#ifdef _MSC_VER
#   include <vld.h>
#endif
#include <vtd/rtti.h>
extern "C"
{
#	include <lua/lua.h>
#	include <lua/lualib.h>
#	include <lua/lauxlib.h>
}
#include <stdio.h>
#include <assert.h>
#define LB_ASSERT assert
#define LB_LOG_W printf
#define LB_LOG_E printf
#include <luabind/luabind.h>


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

void test_rtti()
{
	C* pkC = new C;
	A* pkA = static_cast<A*>(pkC);
	B* pkB = static_cast<B*>(pkC);

	bool b = vtd_is_kind_of(B, pkA);
	b = vtd_is_exact_kind_of(B, pkA);

	A* pkA2 = vtd_dynamic_cast(A, pkB);
	B* pkB2 = vtd_dynamic_cast(B, pkB);
	C* pkC2 = vtd_dynamic_cast(C, pkC);
	C* pkC3 = vtd_dynamic_cast(C, pkB);

	printf("RTTI TEST: %d,%d,%d,%d,%d\n", pkA2->a, pkB2->b, pkC2->c, pkC3->c, b);

	delete pkC;
}

int lua_print(lua_State* L) noexcept
{
	lua_writestring("--->", 4);
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
			return luaL_error(L, "'tostring' must return a string to 'print'");
		if (i > 1) lua_writestring("\t", 1);
		lua_writestring(s, l);
		lua_pop(L, 1);  /* pop result */
	}
	lua_writeline();
	return 0;
}

int main()
{
	test_rtti();
	lua_State* L = luaL_newstate();
	if (L)
	{
		luaL_openlibs(L);
		lua_pushcfunction(L, &lua_print);
		lua_setglobal(L, "print");
		int err = luaL_dofile(L, "startup.lua");
		if (err)
		{
			fprintf(stderr, "ERR>%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		int res = luabind::call_function<int>(L, "luabind_test.func1", 3, 5);
		luabind::call_function(L, "print", "luabind_test.func1(3, 5)=", res);
		res = luabind::call_function<int>(L, "luabind_test.group.func1", 3, 5);
		luabind::call_function(L, "print", "luabind_test.group.func1(3, 5)=", res);

		char input_buf[65536];
		while (true)
		{
			printf("LUA>");
			scanf("%s", input_buf);
			err = luaL_dostring(L, input_buf);
			if (err)
			{
				fprintf(stderr, "ERR>%s\n", lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}
		lua_close(L);
		L = nullptr;
	}
	return 0;
}
