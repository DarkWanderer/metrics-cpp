#include <metrics/registry.h>
#include <metrics/json.h>
#include <metrics/statsd.h>
#include <metrics/timer.h>
#include <metrics/sink.h>
#include <metrics/prometheus.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <map>
#include <thread>
#include <chrono>

using namespace Metrics;
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::this_thread::sleep_for;
using Catch::Matchers::Equals;

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

    auto values = histogram.values();

    REQUIRE(histogram.sum() == 13);
    REQUIRE(histogram.count() == 4);
    REQUIRE(values.size() == 3);
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

    auto values = summary.values();

    REQUIRE(summary.sum() == 11);
    REQUIRE(summary.count() == 4);
    REQUIRE(values.size() == 3);
    REQUIRE(values[0].first == .5);
    REQUIRE(values[1].first == .9);
    REQUIRE(values[2].first == .99);
    REQUIRE(values[0].second == 2);
    REQUIRE(values[1].second == 3);
    REQUIRE(values[2].second == 3);
}

std::shared_ptr<IRegistry> createReferenceRegistry()
{
    auto registry = createRegistry();
    registry->getCounter("counter1") += 1;
    registry->getCounter("counter2", { { "label", "value1" } }) += 1;
    registry->getCounter("counter2", { { "label", "value2" } }) += 2;
    registry->getGauge("gauge1") = 100.;
    registry->getGauge("gauge2", { { "another", "label" } }) = 200.;

    registry->setDescription("counter1", "Description of counter 1");

    registry->getHistogram("histogram1", {}, { 1., 2., 5. }).observe(1).observe(2);
    registry->getHistogram("histogram2", { {"more", "labels"} }, { 1., 2., 5. }).observe(3).observe(4);

    registry->getSummary("summary1").observe(1).observe(2).observe(3);
    registry->getSummary("summary2", { {"summary", "label"} }).observe(3).observe(3).observe(5);

    return registry;
}

TEST_CASE("Registry.Registry", "[registry]")
{
    auto registry = createReferenceRegistry();

    CHECK_THROWS(registry->getGauge("counter1"));
    CHECK_THROWS(registry->getCounter("gauge1"));
    CHECK_THROWS(registry->getHistogram("counter1"));

    auto contains = [](vector<string> v, string key)
    {
        return find(v.begin(), v.end(), key) != v.end();
    };

    auto names = registry->metricNames();

    REQUIRE(contains(names, "counter1"));
    REQUIRE(contains(names, "counter2"));
    REQUIRE(contains(names, "gauge1"));
    REQUIRE(contains(names, "gauge2"));
};

TEST_CASE("Serialize.Prometheus", "[prometheus]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Prometheus::serialize(registry);

    REQUIRE_THAT(result, Equals(R"(# HELP counter1 Description of counter 1
# TYPE counter1 counter
counter1 1
# TYPE counter2 counter
counter2{label="value1"} 1
counter2{label="value2"} 2
# TYPE gauge1 gauge
gauge1 100
# TYPE gauge2 gauge
gauge2{another="label"} 200
# TYPE histogram1 histogram
histogram1{le="1"} 1
histogram1{le="2"} 2
histogram1{le="5"} 2
histogram1_sum 3
histogram1_count 2
# TYPE histogram2 histogram
histogram2{more="labels",le="1"} 0
histogram2{more="labels",le="2"} 0
histogram2{more="labels",le="5"} 2
histogram2_sum{more="labels"} 7
histogram2_count{more="labels"} 2
# TYPE summary1 summary
summary1{quantile="0.5"} 1
summary1{quantile="0.9"} 2
summary1{quantile="0.99"} 2
summary1{quantile="0.999"} 2
summary1_sum 6
summary1_count 3
# TYPE summary2 summary
summary2{summary="label",quantile="0.5"} 3
summary2{summary="label",quantile="0.9"} 3
summary2{summary="label",quantile="0.99"} 3
summary2{summary="label",quantile="0.999"} 3
summary2_sum{summary="label"} 11
summary2_count{summary="label"} 3
)"));
}

TEST_CASE("Serialize.Json", "[json]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Json::serializeJson(registry);

    REQUIRE_THAT(result, Equals(R"([{"name":"counter1","type":"counter","value":1},{"labels":{"label":"value1"},"name":"counter2","type":"counter","value":1},{"labels":{"label":"value2"},"name":"counter2","type":"counter","value":2},{"name":"gauge1","type":"gauge","value":100.0},{"labels":{"another":"label"},"name":"gauge2","type":"gauge","value":200.0},{"buckets":[{"bound":1.0,"count":1},{"bound":2.0,"count":2},{"bound":5.0,"count":2}],"count":2,"name":"histogram1","sum":3.0,"type":"histogram"},{"buckets":[{"bound":1.0,"count":0},{"bound":2.0,"count":0},{"bound":5.0,"count":2}],"count":2,"labels":{"more":"labels"},"name":"histogram2","sum":7.0,"type":"histogram"},{"count":3,"name":"summary1","quantiles":[{"count":1,"quantile":0.5},{"count":2,"quantile":0.9},{"count":2,"quantile":0.99},{"count":2,"quantile":0.999}],"sum":6.0,"type":"summary"},{"count":3,"labels":{"summary":"label"},"name":"summary2","quantiles":[{"count":3,"quantile":0.5},{"count":3,"quantile":0.9},{"count":3,"quantile":0.99},{"count":3,"quantile":0.999}],"sum":11.0,"type":"summary"}])"));
}

TEST_CASE("Serialize.Jsonl", "[jsonl]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Json::serializeJsonl(registry);

    REQUIRE_THAT(result, Equals(R"({"name":"counter1","type":"counter","value":1}
{"labels":{"label":"value1"},"name":"counter2","type":"counter","value":1}
{"labels":{"label":"value2"},"name":"counter2","type":"counter","value":2}
{"name":"gauge1","type":"gauge","value":100.0}
{"labels":{"another":"label"},"name":"gauge2","type":"gauge","value":200.0}
{"buckets":[{"bound":1.0,"count":1},{"bound":2.0,"count":2},{"bound":5.0,"count":2}],"count":2,"name":"histogram1","sum":3.0,"type":"histogram"}
{"buckets":[{"bound":1.0,"count":0},{"bound":2.0,"count":0},{"bound":5.0,"count":2}],"count":2,"labels":{"more":"labels"},"name":"histogram2","sum":7.0,"type":"histogram"}
{"count":3,"name":"summary1","quantiles":[{"count":1,"quantile":0.5},{"count":2,"quantile":0.9},{"count":2,"quantile":0.99},{"count":2,"quantile":0.999}],"sum":6.0,"type":"summary"}
{"count":3,"labels":{"summary":"label"},"name":"summary2","quantiles":[{"count":3,"quantile":0.5},{"count":3,"quantile":0.9},{"count":3,"quantile":0.99},{"count":3,"quantile":0.999}],"sum":11.0,"type":"summary"}
)"));
}

TEST_CASE("Serialize.Statsd", "[statsd]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Statsd::serialize(registry);

    REQUIRE_THAT(result, Equals(R"(counter1|1|c
counter2,label=value1|1|c
counter2,label=value2|2|c
gauge1|100|g
gauge2,another=label|200|g
)"));
}

TEST_CASE("Timer.Counter", "[timer][counter]")
{
    Counter c;
    {
        Timer<milliseconds> t(c);
        sleep_for(2ms);
    }
    REQUIRE(c.value() > 1);
}

TEST_CASE("Timer.Gauge", "[timer][gauge]")
{
    Gauge g;
    {
        Timer<milliseconds> t(g);
        sleep_for(2ms);
    }
    REQUIRE(g.value() > 1);
}

TEST_CASE("Timer.Histogram", "[timer][histogram]")
{
    Histogram h({ 0.1, 100. });
    {
        Timer<milliseconds> t(h);
        sleep_for(2ms);
    }
    REQUIRE(h.sum() > 1);
    REQUIRE(h.count() == 1);
    auto values = h.values();
    REQUIRE(values[0].second == 0);
    REQUIRE(values[1].second == 1);
}

TEST_CASE("Timer.Summary", "[timer][summary]")
{
    Summary s({ .5, .9, .99 });
    {
        Timer<milliseconds> t(s);
        sleep_for(2ms);
    }
    REQUIRE(s.sum() > 1);
    REQUIRE(s.count() == 1);
    auto values = s.values();
    REQUIRE(values[0].second > 1);
    REQUIRE(values[1].second > 1);
    REQUIRE(values[2].second > 1);
}
