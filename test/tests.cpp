#include <metrics/registry.h>
#include <metrics/prometheus.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <map>

using std::string;

TEST_CASE("Labels", "[metric][labels]")
{
    auto l1 = Metrics::Labels{ {"a", "b"}, {"c", "d"} };
    auto l2 = Metrics::Labels{ {"c", "d"}, {"a", "b"} };
    auto l3 = Metrics::Labels{ {"a", "b"}, {"c", "e"} };

    REQUIRE((l1 == l2));
    REQUIRE((l1 != l3));
    REQUIRE((l1 < l3));

    l3["c"] = "d";
    REQUIRE((l1 == l3));

    REQUIRE((Metrics::Labels{ {"a", "a1"}, { "a", "a2" } } == Metrics::Labels{ {"a", "a1"} }));
}

TEST_CASE("Counters", "[metric][counter]")
{
    auto registry = Metrics::createRegistry();
    auto counter = registry->getCounter({ "counter" });
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

TEST_CASE("Gauges", "[metric][gauge]")
{
    auto registry = Metrics::createRegistry();
    auto gauge = registry->getGauge({ "gauge" });
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

TEST_CASE("Histograms", "[metric][histogram]")
{
    auto registry = Metrics::createRegistry();
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

TEST_CASE("Registry", "[registry]")
{
    for (auto& registry : { Metrics::createRegistry(), Metrics::createLargeRegistry() })
    {
        auto c1 = registry->getCounter({ "counter1" });
        auto c2 = registry->getCounter({ "counter2", { { "some", "label" } } });
        auto g1 = registry->getGauge({ "gauge1" });
        auto g2 = registry->getGauge({ "gauge2", {{"other", "label"}} });

        CHECK_THROWS(registry->getGauge({ "counter1" }));
        CHECK_THROWS(registry->getCounter({ "gauge1" }));
        CHECK_THROWS(registry->getHistogram({ "counter1" }));

        auto contains = [](std::vector<Metrics::Key> container, Metrics::Key key)
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
}

using Catch::Matchers::ContainsSubstring;
TEST_CASE("Prometheus", "[prometheus]")
{
    auto registry = Metrics::createRegistry();
    auto c1 = registry->getCounter({ "counter1" });
    auto c2 = registry->getCounter({ "counter2", { { "some", "label" } } });
    auto g1 = registry->getGauge({ "gauge1" });
    auto g2 = registry->getGauge({ "gauge2", {{"other", "label"}} });

    c1 += 5;
    c2 += 10;
    g1 = 123;
    g2 = 321;

    auto result = serializeToPrometheus(*registry);
    REQUIRE_THAT(result, ContainsSubstring("counter1 5"));
    REQUIRE_THAT(result, ContainsSubstring("counter2{some=\"label\"} 10"));
    REQUIRE_THAT(result, ContainsSubstring("gauge1 123"));
    REQUIRE_THAT(result, ContainsSubstring("gauge2{other=\"label\"} 321"));
}
