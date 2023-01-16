#include <metrics/registry.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Counters", "[counter]")
{
    auto &registry = Metrics::createRegistry();
    auto counter = registry->getCounter({"counter"});
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
    auto &registry = Metrics::createRegistry();
    auto gauge = registry->getGauge({"gauge"});
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
    auto &registry = Metrics::createRegistry();
    auto histogram = registry->getHistogram({"histogram"}, {1., 2., 5.});
    histogram.observe(1);
    histogram.observe(2);
    histogram.observe(3);
    histogram.observe(5);
    REQUIRE(histogram.sum() == 11);
    REQUIRE(histogram.count() == 4);
}
