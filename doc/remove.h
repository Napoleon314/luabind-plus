template <class _Ty>
struct object_traits<vtd::intrusive_ptr<_Ty>>
{
	static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

	static constexpr bool can_get = true;

	static constexpr bool can_push = true;

	static constexpr int stack_count = 1;

	static bool test(lua_State* L, int idx) noexcept
	{
		return detail::test_i_ptr<_Ty>(L, idx);
	}

	static vtd::intrusive_ptr<_Ty> get(lua_State* L, int idx) noexcept
	{
		return detail::get_obj<_Ty>(L, idx);
	}

	static int push(lua_State* L, vtd::intrusive_ptr<_Ty> val) noexcept
	{
		auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
		obj->data = val;
		vtd::intrusive_obj<_Ty>::inc(obj->data);
		return 1;
	}

	static vtd::intrusive_ptr<_Ty> make_default() noexcept
	{
		return nullptr;
	}
};

template <class _Ty>
struct object_traits<vtd::intrusive_ptr<_Ty>&>
{
	static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

	static constexpr bool can_get = false;

	static constexpr bool can_push = true;

	static constexpr int stack_count = 1;

	static int push(lua_State* L, vtd::intrusive_ptr<_Ty>& val) noexcept
	{
		auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
		obj->data = val;
		vtd::intrusive_obj<_Ty>::inc(obj->data);
		return 1;
	}
};

template <class _Ty>
struct object_traits<const vtd::intrusive_ptr<_Ty>&>
{
	static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

	static constexpr bool can_get = true;

	static constexpr bool can_push = true;

	static constexpr int stack_count = 1;

	static bool test(lua_State* L, int idx) noexcept
	{
		return detail::test_obj<_Ty, STORAGE_I_PTR>(L, idx);
	}

	static const vtd::intrusive_ptr<_Ty>& get(lua_State* L, int idx) noexcept
	{
		auto obj = (detail::userdata_obj<_Ty, STORAGE_I_PTR>*)lua_touserdata(L, idx);
		LB_ASSERT(obj->info.storage == STORAGE_I_PTR);
		return *(vtd::intrusive_ptr<_Ty>*)&(obj->data);
	}

	static int push(lua_State* L, const vtd::intrusive_ptr<_Ty>& val) noexcept
	{
		auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
		obj->data = val;
		vtd::intrusive_obj<_Ty>::inc(obj->data);
		return 1;
	}
};

template <class _Ty>
struct object_traits<vtd::intrusive_ptr<_Ty>&&>
{
	static_assert(std::is_class<_Ty>::value, "_Ty is not a class or struct.");

	static constexpr bool can_get = false;

	static constexpr bool can_push = true;

	static constexpr int stack_count = 1;

	static int push(lua_State* L, vtd::intrusive_ptr<_Ty>&& val) noexcept
	{
		auto obj = detail::push_obj<_Ty, STORAGE_I_PTR>(L);
		obj->data = val;
		vtd::intrusive_obj<_Ty>::inc(obj->data);
		return 1;
	}
};