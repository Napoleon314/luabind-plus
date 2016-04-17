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
obj = luabind.TestClass1(5,6)
print("obj:get_sum()", obj:get_sum())
obj:test_manual()
--print(obj)
print("obj", obj.a, obj.b)
--obj = luabind.TestClass1(5,6, 7, 8)
--luabind.add(5, 6, 7)
--obj = luabind.TestClass1.create(9, 10)
--print(obj)
