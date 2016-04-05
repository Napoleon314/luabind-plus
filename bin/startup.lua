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

co = coroutine.create(function (a, b)
	print("in co", a, b)
	a,b = coroutine.yield(a + 2, b + 2)
	print("in co2", a, b)
	return a + 2, b + 2
end)

d,e,f = coroutine.resume(co, 3, 5)
print("out co", e, f)
d,e,f = coroutine.resume(co, e, f)
print("out co2", coroutine.resume(co, e, f))
