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

#include "polymorphic.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <random>
class draw {};
class x2 {};

using PolyMorphicObject = polymorphic::object<void(draw, std::ostream&)const>;
using PolyMorphicRef    = polymorphic::ref<void(draw, std::ostream&)const>;

// Use types instead of names
// void draw(std::ostream&) -> void(draw, std::ostream&)

void call_draw( PolyMorphicRef d )
{
	d.call<draw>(std::cout);
}


template <typename T>
void poly_extend(draw, const T& t, std::ostream& os)
{
    os << __PRETTY_FUNCTION__ << " " << t << " (auditor)\n";
}

template <typename T>
void poly_extend(x2, T& t, std::unique_ptr<int>)
{
    std::cout << __PRETTY_FUNCTION__ << " " << t << " (doubler)\n";
    t = t + t;
}

int main()
{

	std::vector<polymorphic::object<
		void(x2,std::unique_ptr<int>),
		void(draw, std::ostream & os) const
		>> objects;


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);

    for (int i = 0; i < 10; ++i)
    {
        auto type_id = dis(gen);

        switch ( type_id ) {
		case 0:
			objects.emplace_back(i);
			break;
		case 1:
			objects.emplace_back(double(i) + double(i) / 10.0);
			break;
		case 2:
			objects.emplace_back(std::to_string(i) + " string");
			break;
		}
	}

	auto o = objects.front();
    o.call<x2>(nullptr);

    PolyMorphicObject co(10);
    auto co1 = co;
    for ( const auto& o : objects)
        call_draw( o );
    return 0;

//	for (auto& o : objects) o.call<x2>(nullptr);
//	for (auto& o : objects) call_draw(o);
}
