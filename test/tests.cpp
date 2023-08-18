#include <metrics/registry.h>
#include <metrics/serialize.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <map>

using namespace Metrics;
using namespace std;
using Catch::Matchers::ContainsSubstring;

TEST_CASE("Metric.Labels", "[metric][labels]")
{
    auto l1 = Labels{ {"a", "b"}, {"c", "d"} };
    auto l2 = Labels{ {"c", "d"}, {"a", "b"} };
    auto l3 = Labels{ {"a", "b"}, {"c", "e"} };

    REQUIRE((l1 == l2));
    REQUIRE((l1 != l3));
    REQUIRE((l1 < l3));

    l3["c"] = "d";
    REQUIRE((l1 == l3));

    REQUIRE((Labels{ {"a", "a1"}, { "a", "a2" } } == Labels{ {"a", "a1"} }));
}

TEST_CASE("Metric.Counter", "[metric][counter]")
{
    Counter counter;
    REQUIRE(counter == 0);
    counter++;
    REQUIRE(counter == 1);

    auto counter2 = counter;
    auto counter3(counter);
    auto counter4{ counter };

    counter += 99;
    REQUIRE(counter == 100);
    REQUIRE(counter2 == 100);
    REQUIRE(counter3 == 100);
    REQUIRE(counter4 == 100);

    counter.reset();
    REQUIRE(counter == 0);
    REQUIRE(counter2 == 0);
    REQUIRE(counter3 == 0);
    REQUIRE(counter4 == 0);
}

TEST_CASE("Metric.Gauge", "[metric][gauge]")
{
    Gauge gauge;
    REQUIRE(gauge == 0);
    gauge = 5.0;

    auto gauge2 = gauge;
    auto gauge3(gauge);
    auto gauge4{ gauge };

    REQUIRE(gauge == 5.0);
    REQUIRE(gauge2 == 5.0);
    REQUIRE(gauge3 == 5.0);
    REQUIRE(gauge4 == 5.0);

    gauge += 3.0;
    REQUIRE(gauge == 8.0);
    gauge -= 5.0;
    REQUIRE(gauge == 3.0);
}

TEST_CASE("Metric.Histogram", "[metric][histogram]")
{
    Histogram histogram({ 1., 2., 5. });
    histogram.observe(1);
    histogram.observe(2);
    histogram.observe(3);
    histogram.observe(7);
    REQUIRE(histogram.sum() == 13);
    REQUIRE(histogram.count() == 4);

    auto values = histogram.values();

    REQUIRE(values[0].first == 1.);
    REQUIRE(values[1].first == 2.);
    REQUIRE(values[2].first == 5.);
    REQUIRE(values[0].second == 1);
    REQUIRE(values[1].second == 2);
    REQUIRE(values[2].second == 3);
}

TEST_CASE("Metric.Summary", "[metric][summary]")
{
    Summary summary({ .5, .9, .99 });
    summary.observe(1);
    summary.observe(2);
    summary.observe(3);
    summary.observe(5);
    REQUIRE(summary.sum() == 11);
    REQUIRE(summary.count() == 4);
}

struct nocopy {
    int i = 0;
    nocopy() noexcept = default;
    nocopy(nocopy&&) noexcept = default;
    nocopy(const nocopy&) = delete;
    ~nocopy() noexcept = default;
};

TEST_CASE("Registry.Registry", "[registry]") {
    auto registry = createRegistry();
    auto c1 = registry->getCounter({ "counter1" });
    auto c2 = registry->getCounter({ "counter2", { { "some", "label" } } });
    auto g1 = registry->getGauge({ "gauge1" });
    auto g2 = registry->getGauge({ "gauge2", {{"other", "label"}} });

    CHECK_THROWS(registry->getGauge({ "counter1" }));
    CHECK_THROWS(registry->getCounter({ "gauge1" }));
    CHECK_THROWS(registry->getHistogram({ "counter1" }));

    auto contains = [](vector<Key> container, Key key)
    {
        for (const auto& k : container)
            if (k == key)
                return true;
        return false;
    };

    auto keys = registry->keys();

    REQUIRE((contains(keys, { "counter1" })));
    REQUIRE((contains(keys, { "counter2", { { "some", "label" } } })));
    REQUIRE((contains(keys, { "gauge1" })));
    REQUIRE((contains(keys, { "gauge2", {{"other", "label"}} })));
}

TEST_CASE("Prometheus.Counter", "[prometheus][counter]")
{
    auto registry = createRegistry();
    registry->getCounter({ "counter1" }) += 5;
    registry->getCounter({ "counter2", { { "some", "label" } } }) += 10;

    auto result = serializePrometheus(*registry);

    REQUIRE_THAT(result, ContainsSubstring("counter1 5"));
    REQUIRE_THAT(result, ContainsSubstring("counter2{some=\"label\"} 10"));
}

TEST_CASE("Prometheus.Gauge", "[prometheus][gauge]")
{
    auto registry = createRegistry();
    registry->getGauge({ "gauge1" }) = 123;
    registry->getGauge({ "gauge2", {{ "other", "label" }} }) = 321;

    auto result = serializePrometheus(*registry);

    REQUIRE_THAT(result, ContainsSubstring("gauge1 123"));
    REQUIRE_THAT(result, ContainsSubstring("gauge2{other=\"label\"} 321"));
}

TEST_CASE("Prometheus.Histogram", "[prometheus][histogram]")
{
    auto registry = createRegistry();
    registry->getHistogram({ "histogram1" }, { 1., 2., 5. }).observe(1).observe(2);
    registry->getHistogram({ "histogram2", {{"more", "labels"}} }, { 1., 2., 5. }).observe(1).observe(2);

    auto result = serializePrometheus(*registry);

    REQUIRE_THAT(result, ContainsSubstring("histogram1{le=\"1\"} 1"));
    REQUIRE_THAT(result, ContainsSubstring("histogram1_sum 3"));
    REQUIRE_THAT(result, ContainsSubstring("histogram1_count 2"));

    REQUIRE_THAT(result, ContainsSubstring("histogram2{more=\"labels\",le=\"5\"} 2"));
    REQUIRE_THAT(result, ContainsSubstring("histogram2_sum{more=\"labels\"} 3"));
    REQUIRE_THAT(result, ContainsSubstring("histogram2_count{more=\"labels\"} 2"));
}
