////////////////////////////////////////////////////////////////////////////
//
//  The MIT License (MIT)
//  Copyright (c) 2016 Albert D Yang
// -------------------------------------------------------------------------
//  Module:      vtd
//  File name:   smart_ptr.h
//  Created:     2016/04/02 by Albert D Yang
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

#include <atomic>

namespace vtd
{
	class ref_obj
	{
	public:
		ref_obj() noexcept
		{
			ref_count.store(0, std::memory_order_relaxed);
		}

		virtual ~ref_obj() noexcept = default;

		void inc() noexcept
		{
			ref_count.fetch_add(1, std::memory_order_relaxed);
		}

		void dec() noexcept
		{
			if (ref_count.fetch_sub(1, std::memory_order_relaxed) == 1)
			{
				delete_this();
			}
		}

		inline size_t get_ref_count() const noexcept
		{
			return ref_count.load(std::memory_order_relaxed);
		}

	protected:
		virtual void delete_this() noexcept
		{
			delete this;
		}

	private:
		std::atomic_int ref_count;
	};

	template <class _Ty>
	class smart_ptr
	{
	public:
		smart_ptr(_Ty* ptr = nullptr) noexcept
			: naked_ptr(ptr)
		{
			if (naked_ptr)
				naked_ptr->inc();
		}

		smart_ptr(const smart_ptr& lptr) noexcept
		{
			naked_ptr = lptr.naked_ptr;
			if (naked_ptr)
				naked_ptr->inc();
		}

		smart_ptr(smart_ptr&& rptr) noexcept
		{
			naked_ptr = rptr.naked_ptr;
			rptr.naked_ptr = nullptr;
		}

		~smart_ptr() noexcept
		{
			if (naked_ptr)
				naked_ptr->dec();
		}

		template <typename _Other>
		operator smart_ptr<_Other>() const noexcept
		{
			return static_cast<_Other*>(naked_ptr);
		}

		operator _Ty* () const noexcept
		{
			return naked_ptr;
		}

		template <typename _Other>
		operator _Other* () const noexcept
		{
			return static_cast<_Other*>(naked_ptr);
		}

		_Ty& operator * () const noexcept
		{
			return *naked_ptr;
		}

		_Ty* operator -> () const noexcept
		{
			return naked_ptr;
		}

		smart_ptr& operator = (const smart_ptr& lptr) noexcept
		{
			if (naked_ptr != lptr.naked_ptr)
			{
				if (naked_ptr)
					naked_ptr->dec();
				naked_ptr = lptr.naked_ptr;
				if (naked_ptr)
					naked_ptr->inc();
			}
			return *this;
		}

		smart_ptr& operator = (smart_ptr&& rptr) noexcept
		{
			naked_ptr = rptr.naked_ptr;
			rptr.naked_ptr = nullptr;
			return *this;
		}

		smart_ptr& operator = (_Ty* ptr) noexcept
		{
			if (naked_ptr != ptr)
			{
				if (naked_ptr)
					naked_ptr->dec();
				naked_ptr = ptr;
				if (naked_ptr)
					naked_ptr->inc();
			}
			return *this;
		}

		bool operator == (_Ty* ptr) const noexcept
		{
			return (naked_ptr == ptr);
		}

		bool operator != (_Ty* ptr) const noexcept
		{
			return (naked_ptr != ptr);
		}

		bool operator == (const smart_ptr& ptr) const noexcept
		{
			return (naked_ptr == ptr.naked_ptr);
		}

		bool operator != (const smart_ptr& ptr) const noexcept
		{
			return (naked_ptr != ptr.naked_ptr);
		}

		_Ty* p() noexcept
		{
			return naked_ptr;
		}

		const _Ty* p() const noexcept
		{
			return naked_ptr;
		}

		_Ty& l() noexcept
		{
			return *naked_ptr;
		}

		const _Ty& l() const noexcept
		{
			return *naked_ptr;
		}

		_Ty&& r() noexcept
		{
			return std::move(*naked_ptr);
		}

		const _Ty&& r() const noexcept
		{
			return std::move(*naked_ptr);
		}

		static const smart_ptr _null;

	protected:
		_Ty* naked_ptr = nullptr;
	};

	template<class _Ty> const smart_ptr<_Ty> smart_ptr<_Ty>::_null = nullptr;
}

#define vtd_smart_ptr(classname)										\
	class classname;													\
	typedef vtd::smart_ptr<classname> classname##_ptr
