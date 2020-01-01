#include "polymorphic.hpp"
#include <benchmark/benchmark.h>
#include <cstdlib>

#ifdef _MSC_VER
#pragma comment(lib,"shlwapi.lib")
#endif

#include "benchmark_imp.h"


void SomeFunction() {}


std::vector<int> GetRandVector() {
	static std::vector<int> vec = []() {
		std::vector<int> vec;
		for (int i = 0; i < 100; ++i) {
			vec.push_back(std::rand());
		}
		return vec;
	}();
	return vec;
}

static void BM_Virtual(benchmark::State& state) {
	auto b = MakeBase();
	auto pb = b.get();
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		benchmark::DoNotOptimize(pb->draw());
	}
}

constexpr int size = 100;
static void BM_VirtualVector(benchmark::State& state) {
	std::vector<std::unique_ptr<Base>> objects;
	for (int i:GetRandVector()) {
		objects.push_back(MakeBaseRand(i));
	}
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		for (auto& o : objects) {
			benchmark::DoNotOptimize(o->draw());
		}
	}
}
static void BM_PolyRef(benchmark::State& state) {
	Dummy d;
	polymorphic::ref<int(draw)> ref(d);
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		benchmark::DoNotOptimize(ref.call<draw>());
	}
	static_assert(sizeof(ref) == 3 * sizeof(void*));
}

static void BM_PolyObject(benchmark::State& state) {
	Dummy d;
	polymorphic::object<int(draw)> ref(d);
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		benchmark::DoNotOptimize(ref.call<draw>());
	}
	static_assert(sizeof(ref) == 4 * sizeof(void*));
}

static void BM_PolyObjectVector(benchmark::State& state) {
	std::vector<polymorphic::object<int(draw)>> objects;
	for (int i:GetRandVector()) {
		objects.push_back(GetObjectRand(i));
	}
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		for (auto& o : objects) {
			benchmark::DoNotOptimize(o.call<draw>());
		}
	}
}

static void BM_PolyRefVector(benchmark::State& state) {
	std::vector<polymorphic::object<int(draw)>> objects;
	for (int i:GetRandVector()) {
		objects.push_back(GetObjectRand(i));
	}

	std::vector<polymorphic::ref<int(draw)>> refs(objects.begin(), objects.end());
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		for (auto& r : refs) {
			benchmark::DoNotOptimize(r.call<draw>());
		}
	}
}

static void BM_Function(benchmark::State& state) {
	auto f = GetFunction();
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		benchmark::DoNotOptimize(f());
	}
}

static void BM_FunctionVector(benchmark::State& state) {
	std::vector<std::function<int()>> objects;
	for (int i:GetRandVector()) {
		objects.push_back(GetFunctionRand(i));
	}
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		for (auto& o : objects) {
			benchmark::DoNotOptimize(o());
		}
	}
}

static void BM_NonVirtual(benchmark::State& state) {
	NonVirtual n;
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		benchmark::DoNotOptimize(n.draw());
	}
}

static void BM_NonVirtualVector(benchmark::State& state) {
	std::vector<NonVirtual> objects;
	for (int i = 0; i < GetRandVector().size(); ++i) {
		objects.push_back(NonVirtual{});
	}
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		for (auto& o : objects) {
			benchmark::DoNotOptimize(o.draw());
		}
	}
}
// Register the function as a benchmark
BENCHMARK(BM_NonVirtual);
BENCHMARK(BM_Virtual);
BENCHMARK(BM_Function);
BENCHMARK(BM_PolyRef);
BENCHMARK(BM_PolyObject);

BENCHMARK(BM_NonVirtualVector);
BENCHMARK(BM_VirtualVector);
BENCHMARK(BM_FunctionVector);
BENCHMARK(BM_PolyRefVector);
BENCHMARK(BM_PolyObjectVector);


BENCHMARK_MAIN();
