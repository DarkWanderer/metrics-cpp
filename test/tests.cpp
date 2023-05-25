#include <metrics/registry.h>

#include <catch2/catch_test_macros.hpp>
#include <map>

using std::string;

TEST_CASE("Labels", "[labels]")
{
	auto l1 = Metrics::Labels{ {"a", "b"}, {"c", "d"} };
	auto l2 = Metrics::Labels{ {"c", "d"}, {"a", "b"} };
	auto l3 = Metrics::Labels{ {"a", "b"}, {"c", "e"} };

	REQUIRE((l1 == l2));
	REQUIRE((l1 != l3));
	REQUIRE((l1 < l3));

	l3["c"] = "d";
	REQUIRE((l1 == l3));

	CHECK_THROWS(Metrics::Labels{ {"a", "a1"}, {"a", "a2"} });
}

TEST_CASE("Counters", "[counter]")
{
	auto registry = Metrics::createRegistry();
	auto counter = registry->getCounter({ "counter" });
	REQUIRE(counter == 0);
	counter++;
	REQUIRE(counter == 1);
	counter += 99;
	REQUIRE(counter == 100);
	counter.reset();
	REQUIRE(counter == 0);
}

TEST_CASE("Gauges", "[gauge]")
{
	auto& registry = Metrics::createRegistry();
	auto gauge = registry->getGauge({ "gauge" });
	REQUIRE(gauge == 0);
	gauge = 5.0;
	REQUIRE(gauge == 5.0);
	gauge += 3.0;
	REQUIRE(gauge == 8.0);
	gauge -= 5.0;
	REQUIRE(gauge == 3.0);
}

TEST_CASE("Histograms", "[histogram]")
{
	auto& registry = Metrics::createRegistry();
	auto histogram = registry->getHistogram({ "histogram" }, { 1., 2., 5. });
	histogram.observe(1);
	histogram.observe(2);
	histogram.observe(3);
	histogram.observe(5);
	REQUIRE(histogram.sum() == 11);
	REQUIRE(histogram.count() == 4);
}

struct nocopy {
	int i = 0;
	nocopy() noexcept = default;
	nocopy(nocopy&&) noexcept = default;
	nocopy(const nocopy&) = delete;
	~nocopy() noexcept = default;
};

TEST_CASE("Emplace", "[emplace]")
{
	std::map<int, nocopy> m;
	std::vector<std::pair<int, nocopy>> v;

	m.emplace(std::move(std::make_pair<int, nocopy>(1, {})));
	v.emplace_back(std::move(std::make_pair<int, nocopy>(1, {})));
}
