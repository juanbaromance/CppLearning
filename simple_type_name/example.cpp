// Copyright 2019 Google LLC
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
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include "simple_type_name.h"

namespace test{
struct MyClass;
}

int main() {
  std::cout << simple_type_name::short_name<test::MyClass> << "\n";
  std::cout << simple_type_name::long_name<test::MyClass>  << "\n";

  static_assert(simple_type_name::short_name<int> == "int");
  static_assert(simple_type_name::short_name<class MyClass> == "MyClass");
}