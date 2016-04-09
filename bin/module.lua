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