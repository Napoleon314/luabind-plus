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
