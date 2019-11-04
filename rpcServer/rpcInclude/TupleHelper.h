#pragma once
#include <tuple>


class _TupleHelper
{
private:
	template<size_t N> struct Apply {
		template<typename F, typename T, typename... A>
		static inline auto apply(F && f, T && t, A &&... a)
			-> decltype(Apply<N - 1>::apply(std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...))
		{
			return Apply<N - 1>::apply(std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...);
		}
	};
	template<> struct Apply<0> {
		template<typename F, typename T, typename... A>
		static inline auto apply(F && f, T &&, A &&... a)-> decltype(std::forward<F>(f)(std::forward<A>(a)...))
		{
			return std::forward<F>(f)(std::forward<A>(a)...);
		}
	};

	template<typename Tuple, typename Func, size_t N>
	class ForeachTuple
	{
	public:
		static void foreach(Tuple&& tuple, Func&& func)
		{
			ForeachTuple<Tuple, Func, N - 1>::foreach(std::forward<Tuple>(tuple), std::forward<Func>(func));
			std::forward<Func>(func)(std::get<N - 1>(std::forward<Tuple>(tuple)));
		}
	};
	template<typename Tuple, typename Func>
	class ForeachTuple<Tuple, Func, 1>
	{
	public:
		static void foreach(Tuple&& tuple, Func&& func)
		{
			std::forward<Func>(func)(std::get<0>(std::forward<Tuple>(tuple)));
		}
	};

public:
	// 函数调用
	template<typename Tuple, typename Func>
	auto runFunc(Tuple&& tuple, Func&& func)
		-> decltype(Apply< std::tuple_size<typename std::decay<Tuple>::type>::value>
		::apply(std::forward<Func>(func), std::forward<Tuple>(tuple)))
	{
		return Apply< std::tuple_size<typename std::decay<Tuple>::type>::value>
			::apply(std::forward<Func>(func), std::forward<Tuple>(tuple));
	}

	// 遍历
	template<typename Tuple, typename Func>
	void foreach(Tuple&& tuple, Func&& func)
	{
		ForeachTuple<decltype(tuple), Func, std::tuple_size<typename std::decay<Tuple>::type>::value>
			::foreach(std::forward<Tuple>(tuple), std::forward<Func>(func));
	}

	// 获取元素数量
	template<typename Tuple>
	size_t count(Tuple&& tuple)
	{
		return std::tuple_size<typename std::decay<Tuple>::type>::value;
	}

	static _TupleHelper* getInstance()
	{
		static _TupleHelper helper;
		return &helper;
	}
};
#define TupleHelper _TupleHelper::getInstance()





