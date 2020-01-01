// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// limitations under the License.

#include <algorithm>
#include <iostream>
#include <vector>
#include "tafn.hpp"
#include <optional>


struct multiply {};
struct add {};

void tafn_customization_point(multiply, tafn::type<int>, int& i, int value) {
	i *= value;
}
int tafn_customization_point(add, tafn::type<int>, const int& i, int value) {
	return i + value;
}

namespace algs {
	struct sort {};
	struct unique {};
	struct copy {};
	template <size_t I>
	struct get {};

	template <typename T>//,typename = std::void_t<decltype(std::begin(std::declval<T>()))>>
	decltype(auto) tafn_customization_point(sort, tafn::all_types, T&& t) {
		std::sort(t.begin(), t.end());
		return std::forward<T>(t);
	}
	template <typename T, typename = std::void_t<decltype(std::begin(std::declval<T>()))>>
	decltype(auto) tafn_customization_point(unique, tafn::all_types, T&& t) {
		auto iter = std::unique(t.begin(), t.end());
		t.erase(iter, t.end());
		return std::forward<T>(t);
	}
	template <typename T, typename I, typename = std::void_t<decltype(std::begin(std::declval<T>()))>>
	decltype(auto) tafn_customization_point(copy, tafn::all_types, T&& t, I iter) {
		std::copy(t.begin(), t.end(), iter);
		return std::forward<T>(t);
	}
	template <typename T, size_t I>
	decltype(auto) tafn_customization_point(get<I>, tafn::all_types, T&& t) {
		using std::get;
		return get<I>(std::forward<T>(t));
	}

}  // namespace algs

template <typename T>
void sort_unique(T&& t) {
	using tafn::_;
	t* _<algs::sort>()*_<algs::unique>();
}

#include <system_error>

struct dummy {};

struct operation1 {};
struct operation2 {};

dummy& tafn_customization_point(operation1, tafn::type<dummy>, dummy& d, int i,
	std::error_code& ec) {
	if (i == 2) {
		ec = std::make_error_code(std::errc::function_not_supported);
	}
	return d;
}


void tafn_customization_point(operation2, tafn::type<dummy>, dummy& d, int i,
	int j, std::error_code& ec) {
	if (j == 2) {
		ec = std::make_error_code(std::errc::function_not_supported);
	}
}

template <typename F, typename... Args, typename = std::enable_if_t<!std::disjunction_v<std::is_same<std::error_code,std::decay_t<Args>>...>>, typename =
	std::enable_if_t<tafn::is_action_tag_invocable_v<F, dummy&, Args..., std::error_code&>>
>
decltype(auto) tafn_customization_point(tafn::all_functions<F>,
	tafn::type<dummy>, dummy& d, Args&&... args) {
	using tafn::_;
	std::error_code ec;

	auto call = [&]() mutable {
		return d * _<F>(std::forward<Args>(args)..., ec);
	};

	using V = decltype(call());
	if constexpr (!std::is_same_v<void, V>) {
		V r = call();
		if (ec) throw std::system_error(ec);
		return r;
	}
	else {
		call();
		if (ec) throw std::system_error(ec);
	}
}

void test_exception() {
	using tafn::_;
	try {
		dummy d;
		std::error_code ec;
		d*_<operation1>(1, ec)*_<operation2>(2, 2);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}
}

#include <string>
#include <string_view>

struct get_all_lines {};

std::vector<std::string> tafn_customization_point(get_all_lines, tafn::all_types, std::istream& is) {
	std::vector<std::string> result;
	std::string line;
	while (std::getline(is, line)) {
		result.push_back(line);
	}
	return result;
}

struct output {};
template<typename T,
	typename =
	std::void_t<decltype(std::declval<std::ostream&>() << std::forward<T>(std::declval<T>()))
	>>
	void tafn_customization_point(output, tafn::all_types, const T& t, std::ostream& os, std::string_view delimit = "") {
	os << t << delimit;
}

template<typename T>
struct c_value_type {
	using D = std::decay_t<T>;
	using type = typename D::value_type;
};

template<typename T>
using c_value_type_t = typename c_value_type<T>::type;

template<typename F>
struct call_for_each {};
template<typename F, typename C, typename... Args, typename =
	std::enable_if_t<
	tafn::is_action_tag_invocable_v<F, decltype(*std::forward<C>(std::declval<C>()).begin()), Args...>>>
	void tafn_customization_point(call_for_each<F>, tafn::all_types, C&& c, Args&&... args) {
	using tafn::_;
	for (auto&& v : std::forward<C>(c)) {
		v * _<F>(std::forward<Args>(args)...);
	}
}

namespace smart_reference {
	template<typename T>
	struct reference {
		T* t;
	};

	template<typename F, typename R, typename T, typename... Args, typename = std::enable_if_t<tafn::is_action_tag_invocable_v<F, R&, Args...>>>
	decltype(auto) tafn_customization_point(tafn::all_functions<F>, tafn::type<reference<R>>, T&& t, Args&&... args) {
		using tafn::_;
		return *t.t * _<F>(std::forward<Args>(args)...);
	}

	void test() {
		using tafn::_;
		std::vector<int> v{ 4,3,2,1,2,3 };
		reference<std::vector<int>> a{ &v };
		a * _<algs::sort>() * _<algs::unique>() * _<call_for_each<output>>(std::cout, "\n");


	}
}


namespace simple {

	struct get_data {};

	class my_class {
		std::string data_;
		template<typename T>
		friend decltype(auto) tafn_customization_point(get_data, tafn::type<my_class>, T&& t) {
			return (std::forward<T>(t).data_);
		}
	};




	template<typename T>
	struct smart_reference {
		T& r_;
	};

	template<typename F, typename T, typename Self, typename... Args, typename = std::enable_if_t<tafn::is_action_tag_invocable_v<F, T&, Args...>>>
	decltype(auto) tafn_customization_point(tafn::all_functions<F>, tafn::type<smart_reference<T>>, Self&& self, Args&&... args) {
		using tafn::_;
		return  std::forward<Self>(self).r_ * _<F>(std::forward<Args>(args)...);
	}


	namespace my_methods {

		struct to_string {};

		std::string tafn_customization_point(to_string, tafn::type<int>, int i) {
			return std::to_string(i);
		}

	}

	void test() {
		using tafn::_;
		my_class c;
		c *_<get_data>() = "hello world";
		const my_class& cc = c;
		const auto& str = cc * _<get_data>();
		std::string& s = c * _<get_data>();

		smart_reference<my_class> ref{ c };
		std::string&& rref_str = std::move(c) * _<get_data>();
		std::string& cref_str = ref * _<get_data>();


		5 * _<my_methods::to_string>();



	}

}



#include <iterator>
#include <tuple>
int main() {

	using tafn::_;

	int i{ 5 };

	i * _<multiply>(10);

	std::cout << i;

	int j = 9;
	auto k = i * _<add>(2)*_<add>(3);
	std::cout << k << " " << (j * _<add>(4) * _<add>(5)) << " ";

	std::vector<int> v{
		4, 4, 1, 2, 2, 9, 9, 9, 7, 6, 6,
	};
	v * _<algs::sort>()*_<algs::unique>()*_<call_for_each<output>>(std::cout, "\n");
	sort_unique(v);

	auto t = std::tuple<int, char, int>{ 1, 2, 3 };
	std::cout << (t*_<algs::get<2>>());

	test_exception();


	//std::cin | _<get_all_lines>() | _<algs::sort>() | _<algs::unique>() | _<call_for_each<output>>(std::cout, "\n");
	std::cin * _<get_all_lines>() * _<algs::sort>() * _<algs::unique>() * _<call_for_each<output>>(std::cout, "\n");

	// std::cin.<get_all_lines>().<algs::sort>().<algs::unique>().<call_for_each<output>>(std::cout, "\n");
}
