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

void test_no_return() noexcept
{

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
	TestClass1(int _b) noexcept
	{
		b = _b;
	}

	TestClass1(int _a, int _b) noexcept
	{
		a = _a;
		b = _b;
	}

	virtual ~TestClass1() noexcept
	{

	}

	int get_sum(int c) noexcept
	{
		return a + b + c;
	}

	int b = 0;

};

std::unique_ptr<TestClass1> create(int a, int b) noexcept
{
	return std::unique_ptr<TestClass1>(new TestClass1(a, b));
}

int Test1::val0 = 40;

int test_manual_member(lua_State* L) noexcept
{
	return 0;
}

struct TestA : virtual vtd::ref_obj
{
	int a1 = 5, a2 = 6;
	const int a3 = 7;
};

struct TestB : virtual vtd::ref_obj
{
	int b1 = 7, b2 = 8;
};

struct TestC : virtual vtd::ref_obj
{
	int c1 = 9, c2 = 10;
};

struct TestD : TestA, TestB, TestC
{
	int d1 = 11, d2 = 12;

	int p1()
	{
		return d1 + d2;
	}

	bool setp(int p)
	{
		return true;
	}
};

void TestCovert(TestA a1, vtd::intrusive_ptr<TestA> a2, std::shared_ptr<TestA> a3) noexcept
{

}

int test_val = 15;
const int test_val2 = 16;

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
			def("test_val", test_val),
			def("test_val2", test_val2),
			def_manual("print", &lua_print, 1, 2),
			def("TestCovert", &TestCovert),
			def("add", &add),
			def("add", &add, 1),
			def("add", &add, 2, 3),
			def("test", &test, std::make_tuple(1, 2.0f), 3),
			def("test_no_return", &test_no_return),
			def_const("CONST_VAL", 5),
			def_reader("test_reader2", &get_reader2),
			def_readonly("test_reader3", test_reader3),
			def_readwrite("test_reader", test_reader),
			def_writer<int>("test_reader2", [&](int val) noexcept
			{
				test_reader2 = val;
				return true;
			}),

			class_<Test1>("Test1").
            def_readonly("a", &Test1::a),

			class_<TestClass1, Test1>("TestClass1").
			def("get_sum", &TestClass1::get_sum).
			def("get_sum", &TestClass1::get_sum, 35).
			def_manual("test_manual", &test_manual_member, 1, 2, 3).
			def_manual("test_manual2", &test_manual_member, 1, 2, 3).
			def_readonly("b", &TestClass1::b).
			def(constructor<int, int>()).
			def(constructor<int>(), 1)[
				def_readwrite("val0", Test1::val0),
				def("create", &create)
			],

			class_<TestA>("TestA").
			def(constructor<>()).
			def("inc", &TestA::inc).
			def("a1", &TestA::a1).
			def("a2", &TestA::a2).
			def("a3", &TestA::a3),

			class_<TestB>("TestB").
			def(constructor<>()).
			def("inc", &TestB::inc).
			def_readonly("b1", &TestB::b1).
			def_readonly("b2", &TestB::b2),

			class_<TestC>("TestC").
			def(constructor<>()).
			def("inc", &TestC::inc).
			def_readonly("c1", &TestC::c1).
			def_readonly("c2", &TestC::c2).
			def_writeonly("c1", &TestC::c1),

			class_<TestD, TestA, TestB, TestC>("TestD").
			def(constructor<>()).
			def_readonly("d1", &TestD::d1).
			def_readonly("d2", &TestD::d2).
			def_reader("p1", &TestD::p1).
			def_writeonly("d1", &TestD::d1).
			def_writer("p1", &TestD::setp)

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

		//TestClass1 aaa(5, 6);
		std::shared_ptr<TestClass1> aaa(new TestClass1(7, 8));
	
		type_traits<std::weak_ptr<TestClass1>>::push(L, aaa);
		aaa = nullptr;

		//bool bbb = type_traits<TestClass1&>::test(L, -1);

		auto ptr = type_traits<std::shared_ptr<TestClass1>>::get(L, -1);

		{
			//TestClass1* obj = new TestClass1(7, 8);
			//auto aaa = &TestClass1::get_sum;
			//bool t = std::is_same<decltype(aaa), func_1>::value;
			//func_1 = aaa;
			//(obj->*aaa)();
			//(((C*)pvSelf)->*pfuncCall)();
			//auto a = member_func_invoke(obj, aaa);


			//delete obj;
		}
		

		//TestClass1& bbb = type_traits<TestClass1&>::get(L, -1);
		lua_pop(L, 1);

		

		//decltype(aaa) bbb(aaa);

		

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
