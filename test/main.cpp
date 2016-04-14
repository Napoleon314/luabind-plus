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

	bool b = vtd_is_kind_of<B>(pkA);
	b = vtd_is_exact_kind_of<B>(pkA);

	A* pkA2 = vtd_dynamic_cast<A>(pkB);
	B* pkB2 = vtd_dynamic_cast<B>(pkB);
	C* pkC2 = vtd_dynamic_cast<C>(pkC);
	C* pkC3 = vtd_dynamic_cast<C>(pkB);

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

int add(int a, int b) noexcept
{
	return a + b;
}

std::tuple<int,int> test(std::tuple<int, float> a, int b) noexcept
{
	return std::make_tuple(std::get<0>(a), b);
}

enum class TestEnum
{
	test_enum1,
	test_enum2,
	test_enum3
};

int test_reader = 15;

int test_reader2 = 30;

int test_reader3 = 60;

int get_reader2() noexcept
{
	return test_reader2;
}

int reader(lua_State* L) noexcept
{
	lua_pushinteger(L, test_reader);
	return 1;
}

int writer(lua_State* L) noexcept
{
	test_reader = luabind::type_traits<int>::get(L, 1);
	return 0;
}

class Test1
{
public:
    static int val0;
    
    int a = 0;
    
};

class TestClass1 : public Test1
{
public:
	int b = 0;

};

int Test1::val0 = 40;

int main()
{
	using namespace std;
	using namespace luabind;
	test_rtti();
	lua_State* L = luaL_newstate();
	if (L)
	{
		luaL_openlibs(L);

		module(L)[
			namespace_("luabind")[scope()]
		];

		module(L, "luabind")[
			namespace_("luabind")[scope()]
		];

		module(L, "luabind")[
			def_manual("print", &lua_print, 1, 2),
			def("add", &add),
			def("add", &add, 1),
			def("add", &add, 2, 3),
			def("test", &test, std::make_tuple(1, 2.0f), 3),
			def_const("CONST_VAL", 5),
			def_reader("test_reader2", &get_reader2),
			def_readonly("test_reader3", test_reader3),
			def_readwrite("test_reader", test_reader),
			def_writer<int>("test_reader2", [&](int val) noexcept
			{
				test_reader2 = val;
			}),
			class_<Test1>("Test1")[
				def_readwrite("val0", Test1::val0)
			],
			class_<TestClass1, Test1>("TestClass1").
                def(constructor<int>()).
                def(constructor<int,int>())

			//def_manual_writer("test_reader", &writer)
		];

		lua_pushcfunction(L, &lua_print);
		lua_setglobal(L, "print");
		int err(0);
		
		/*err = luaL_dofile(L, "startup.lua");
		if (err)
		{
			fprintf(stderr, "ERR>%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		int res = call_function<int>(L, "luabind_test.func1", 3, 5);
		call_function(L, "print", "luabind_test.func1(3, 5)=", res);
		res = call_function<int>(L, "luabind_test.group.func1", 3, 5);
		call_function(L, "print", "luabind_test.group.func1(3, 5)=", res);		

		object o = globals(L)["t"];

		o.foreach([](lua_State* L) noexcept
		{
			LUABIND_HOLD_STACK(L);
			push_func_name(L, "print");
			lua_pushvalue(L, -3);
			lua_pushvalue(L, -3);
			lua_pcall(L, 2, 1, 0);
		});*/
		
		err = luaL_dofile(L, "module.lua");
		if (err)
		{
			fprintf(stderr, "ERR>%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		static_assert(count_func_params(&add) == 2, "");

		/*char input_buf[65536];
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
		}*/	

		//pair<float, tuple<const char*, int, int>>& bbb = *(pair<float, tuple<const char*, int, int>>*)&ttt;

		//bool bbb = params_checker<int, char, float>::targets<char, float>::is_matched;

		lua_close(L);
		L = nullptr;
	}
	return 0;
}
