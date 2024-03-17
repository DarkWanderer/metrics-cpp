#include <metrics/registry.h>
#include <metrics/json.h>
#include <metrics/statsd.h>
#include <metrics/timer.h>
#include <metrics/sink.h>
#include <metrics/prometheus.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/catch_approx.hpp>

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

    CHECK((l1 == l2));
    CHECK((l1 != l3));
    CHECK((l1 < l3));

    l3["c"] = "d";
    CHECK((l1 == l3));

    CHECK((Labels{ {"a", "a1"}, { "a", "a2" } } == Labels{ {"a", "a1"} }));
}

TEST_CASE("Metric.Counter", "[metric][counter]")
{
    Counter counter;
    CHECK(counter == 0);
    counter++;
    CHECK(counter == 1);

    auto counter2 = counter;
    auto counter3(counter);
    auto counter4{ counter };

    counter += 99;
    CHECK(counter == 100);
    CHECK(counter2 == 100);
    CHECK(counter3 == 100);
    CHECK(counter4 == 100);

    counter.reset();
    CHECK(counter == 0);
    CHECK(counter2 == 0);
    CHECK(counter3 == 0);
    CHECK(counter4 == 0);
}

TEST_CASE("Metric.Gauge", "[metric][gauge]")
{
    Gauge gauge;
    CHECK(gauge == 0);
    gauge = 5.0;

    auto gauge2 = gauge;
    auto gauge3(gauge);
    auto gauge4{ gauge };

    CHECK(gauge == 5.0);
    CHECK(gauge2 == 5.0);
    CHECK(gauge3 == 5.0);
    CHECK(gauge4 == 5.0);

    gauge += 3.0;
    CHECK(gauge == 8.0);
    gauge -= 5.0;
    CHECK(gauge == 3.0);
}

TEST_CASE("Metric.Histogram", "[metric][histogram]")
{
    Histogram histogram({ 1., 2., 5. });
    histogram.observe(1);
    histogram.observe(2);
    histogram.observe(3);
    histogram.observe(7);

    auto values = histogram.values();

    CHECK(histogram.sum() == 13);
    CHECK(histogram.count() == 4);
    CHECK(values.size() == 3);
    CHECK(values[0].first == 1.);
    CHECK(values[1].first == 2.);
    CHECK(values[2].first == 5.);
    CHECK(values[0].second == 1);
    CHECK(values[1].second == 2);
    CHECK(values[2].second == 3);
}

TEST_CASE("Metric.Summary", "[metric][summary]")
{
    const vector<double> expected_quantiles = { .5, .75, .99 };
    const vector<uint64_t> expected_values = { 2,3,5 };

    Summary summary(expected_quantiles);
    summary.observe(1);
    summary.observe(1);
    summary.observe(2);
    summary.observe(3);
    summary.observe(5);

    vector<double> actual_quantiles;
    vector<uint64_t> actual_values;

    for (auto v : summary.values()) {
        actual_quantiles.push_back(v.first);
        actual_values.push_back(v.second);
    };

    CHECK(actual_quantiles == expected_quantiles);
    CHECK(actual_values == expected_values);
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

    CHECK(contains(names, "counter1"));
    CHECK(contains(names, "counter2"));
    CHECK(contains(names, "gauge1"));
    CHECK(contains(names, "gauge2"));
};

TEST_CASE("Serialize.Prometheus", "[prometheus]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Prometheus::serialize(registry);

    CHECK_THAT(result, Equals(R"(# HELP counter1 Description of counter 1
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
summary1{quantile="0.5"} 2
summary1{quantile="0.9"} 3
summary1{quantile="0.99"} 3
summary1{quantile="0.999"} 3
summary1_sum 6
summary1_count 3
# TYPE summary2 summary
summary2{summary="label",quantile="0.5"} 3
summary2{summary="label",quantile="0.9"} 5
summary2{summary="label",quantile="0.99"} 5
summary2{summary="label",quantile="0.999"} 5
summary2_sum{summary="label"} 11
summary2_count{summary="label"} 3
)"));
}

TEST_CASE("Serialize.Json", "[json]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Json::serializeJson(registry);

    CHECK_THAT(result, Equals(R"([{"name":"counter1","type":"counter","value":1},{"name":"counter2","labels":{"label":"value1"},"type":"counter","value":1},{"name":"counter2","labels":{"label":"value2"},"type":"counter","value":2},{"name":"gauge1","type":"gauge","value":1E2},{"name":"gauge2","labels":{"another":"label"},"type":"gauge","value":2E2},{"name":"histogram1","type":"histogram","count":2,"sum":3E0,"buckets":[{"bound":1E0,"count":1},{"bound":2E0,"count":2},{"bound":5E0,"count":2}]},{"name":"histogram2","labels":{"more":"labels"},"type":"histogram","count":2,"sum":7E0,"buckets":[{"bound":1E0,"count":0},{"bound":2E0,"count":0},{"bound":5E0,"count":2}]},{"name":"summary1","type":"summary","count":3,"sum":6E0,"quantiles":[{"quantile":5E-1,"count":2},{"quantile":9E-1,"count":3},{"quantile":9.9E-1,"count":3},{"quantile":9.99E-1,"count":3}]},{"name":"summary2","labels":{"summary":"label"},"type":"summary","count":3,"sum":1.1E1,"quantiles":[{"quantile":5E-1,"count":3},{"quantile":9E-1,"count":5},{"quantile":9.9E-1,"count":5},{"quantile":9.99E-1,"count":5}]}])"));
}

TEST_CASE("Serialize.Jsonl", "[jsonl]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Json::serializeJsonl(registry);

    CHECK_THAT(result, Equals(R"({"name":"counter1","type":"counter","value":1}
{"name":"counter2","labels":{"label":"value1"},"type":"counter","value":1}
{"name":"counter2","labels":{"label":"value2"},"type":"counter","value":2}
{"name":"gauge1","type":"gauge","value":1E2}
{"name":"gauge2","labels":{"another":"label"},"type":"gauge","value":2E2}
{"name":"histogram1","type":"histogram","count":2,"sum":3E0,"buckets":[{"bound":1E0,"count":1},{"bound":2E0,"count":2},{"bound":5E0,"count":2}]}
{"name":"histogram2","labels":{"more":"labels"},"type":"histogram","count":2,"sum":7E0,"buckets":[{"bound":1E0,"count":0},{"bound":2E0,"count":0},{"bound":5E0,"count":2}]}
{"name":"summary1","type":"summary","count":3,"sum":6E0,"quantiles":[{"quantile":5E-1,"count":2},{"quantile":9E-1,"count":3},{"quantile":9.9E-1,"count":3},{"quantile":9.99E-1,"count":3}]}
{"name":"summary2","labels":{"summary":"label"},"type":"summary","count":3,"sum":1.1E1,"quantiles":[{"quantile":5E-1,"count":3},{"quantile":9E-1,"count":5},{"quantile":9.9E-1,"count":5},{"quantile":9.99E-1,"count":5}]}
)"));
}

TEST_CASE("Serialize.Statsd", "[statsd]")
{
    auto registry = createReferenceRegistry();
    auto result = Metrics::Statsd::serialize(registry);

    CHECK_THAT(result, Equals(R"(counter1|1|c
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
    CHECK(c.value() > 1);
}

TEST_CASE("Timer.Gauge", "[timer][gauge]")
{
    Gauge g;
    {
        Timer<milliseconds> t(g);
        sleep_for(2ms);
    }
    CHECK(g.value() > 1);
}

TEST_CASE("Timer.Histogram", "[timer][histogram]")
{
    Histogram h({ 0.1, 100. });
    {
        Timer<milliseconds> t(h);
        sleep_for(2ms);
    }
    CHECK(h.sum() > 1);
    CHECK(h.count() == 1);
    auto values = h.values();
    CHECK(values[0].second == 0);
    CHECK(values[1].second == 1);
}

TEST_CASE("Timer.Summary", "[timer][summary]")
{
    Summary s({ .5, .9, .99 });
    {
        Timer<milliseconds> t(s);
        sleep_for(2ms);
    }
    CHECK(s.sum() > 1);
    CHECK(s.count() == 1);
    auto values = s.values();
    CHECK(values[0].second > 1);
    CHECK(values[1].second > 1);
    CHECK(values[2].second > 1);
}

TEST_CASE("Sink.OnDemand", "[url][sink]")
{
    vector<string> urls = {
        "statsd+tcp://localhost:1234/",

        "statsd+udp://localhost:1234/",

        "pushgateway+http://pushgateway.example.org:9091/metrics/job/some_job/instance/some_instance",
        "pushgateway+https://pushgateway.example.org:9091/metrics/job/some_job/instance/some_instance",
    };

    for (const auto& url : urls) {
        DYNAMIC_SECTION("url=" << url) {
            auto sink = createOnDemandSink(url);
            CHECK(sink != nullptr);
        }
    }
}

TEST_CASE("Sink.Registry", "[url][sink]")
{
    vector<string> urls = {
        "prometheus+http://127.0.0.1:8080",
    };

    for (const auto& url : urls) {
        DYNAMIC_SECTION("url=" << url) {
            auto sink = createRegistrySink(nullptr, url);
            CHECK(sink != nullptr);
        }
    }
}
