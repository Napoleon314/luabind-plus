print("startup.lua loaded")

luabind_test = {}
luabind_test.group = {}

function luabind_test.func1(a, b)
	return a + b
end

function luabind_test.group.func1(a, b)
	return a * b
end

function pcallk_test()
	print("pcallk test1")
	return 0
end

a = 1
b = "ttt"
c = 88

t = { 1, 2 , 3 }

t.a = "abc"
t.b = "def"