--obj = luabind.TestClass1.new_s(88, 99)
--print("obj", obj, obj.b)

function test_lua_func()
	return 5
end

d = luabind.TestD()
print("d.d1,d.d2", d.d1, d.d2)

test_obj = { super = d }
test_meta = {}
test_meta.__index = d
test_meta.__newindex = d

setmetatable(test_obj, test_meta)

test_obj.d1 = 100

print("test_obj", test_obj.d1, test_obj.super.d1)



print(luabind.EnumTest)
print(luabind.ENUM_1, luabind.ENUM_2, luabind.ENUM_3)
print(luabind.EnumTest.e1, luabind.EnumTest.e2, luabind.EnumTest.e3)

a1 = luabind.TestA()
a2 = luabind.TestD.new()
a3 = luabind.TestA.new_s()

luabind.TestConvert(a1, a2, a3)

luabind.TestClass1.val0 = 78
print(luabind.TestClass1, luabind.TestClass1.val0)
--luabind.TestClass1.a = 5
luabind.test_reader = 50
luabind.test_reader2 = 80
luabind.ttt = 6

print(luabind.test_reader, luabind.ttt)
print(luabind.test_reader2)

a = 0
print(a)

--luabind = 0
luabind.a = 0
--luabind.luabind = 0

print(luabind, luabind.CONST_VAL)
--luabind.CONST_VAL = 6
print(_G, luabind.luabind)
luabind.print(1,2)
print("luabind.add=", luabind.add(5))
print("luabind.test=", luabind.test(5, 5.5, 8))

print(luabind.test_reader)
print(luabind.test_reader2)
print(luabind.test_reader3)
--luabind.test_reader = 9

--obj = luabind.TestClass1()
--obj = luabind.TestClass1(5)
--obj = luabind.TestClass1(5,6)
--print("obj:get_sum()", obj:get_sum())
--obj:test_manual()
--print(obj)
--print("obj", obj.a, obj.b)
--obj = luabind.TestClass1(5,6, 7, 8)
--luabind.add(5, 6, 7)
--obj = luabind.TestClass1.create(9, 10)
--print(obj)

obj = luabind.TestD()
obj.a1 = 88
obj.a2 = 99
print("a1,a2,a3", obj.a1, obj.a2, obj.a3)
print("b1,b2", obj.b1, obj.b2)
print("c1,c2", obj.c1, obj.c2)
print("d1,d2", obj.d1, obj.d2)
print("p1", obj.p1)
--print(luabind.TestD.ddd)
obj.d1 = 9
obj.c1 = 15
print(obj.d1, obj.c1)
obj.p1 = 99