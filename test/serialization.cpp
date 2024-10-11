#include <metrics/registry.h>
#include <metrics/json.h>
#include <metrics/statsd.h>
#include <metrics/timer.h>
#include <metrics/sink.h>
#include <metrics/prometheus.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "common.hpp"

#include <map>
#include <thread>
#include <chrono>

using namespace Metrics;
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::this_thread::sleep_for;
using Catch::Matchers::Equals;

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
histogram1{le="inf"} 2
histogram1_sum 3
histogram1_count 2
# TYPE histogram2 histogram
histogram2{more="labels",le="1"} 0
histogram2{more="labels",le="2"} 0
histogram2{more="labels",le="5"} 2
histogram2{more="labels",le="inf"} 2
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

    CHECK_THAT(result, Equals(R"([{"name":"counter1","type":"counter","value":1},{"name":"counter2","labels":{"label":"value1"},"type":"counter","value":1},{"name":"counter2","labels":{"label":"value2"},"type":"counter","value":2},{"name":"gauge1","type":"gauge","value":1E2},{"name":"gauge2","labels":{"another":"label"},"type":"gauge","value":2E2},{"name":"histogram1","type":"histogram","sum":3E0,"count":2,"buckets":[{"bound":1E0,"count":1},{"bound":2E0,"count":2},{"bound":5E0,"count":2},{"bound":1e99999,"count":2}]},{"name":"histogram2","labels":{"more":"labels"},"type":"histogram","sum":7E0,"count":2,"buckets":[{"bound":1E0,"count":0},{"bound":2E0,"count":0},{"bound":5E0,"count":2},{"bound":1e99999,"count":2}]},{"name":"summary1","type":"summary","count":3,"sum":6E0,"quantiles":[{"quantile":5E-1,"count":2},{"quantile":9E-1,"count":3},{"quantile":9.9E-1,"count":3},{"quantile":9.99E-1,"count":3}]},{"name":"summary2","labels":{"summary":"label"},"type":"summary","count":3,"sum":1.1E1,"quantiles":[{"quantile":5E-1,"count":3},{"quantile":9E-1,"count":5},{"quantile":9.9E-1,"count":5},{"quantile":9.99E-1,"count":5}]}])"));
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
{"name":"histogram1","type":"histogram","sum":3E0,"count":2,"buckets":[{"bound":1E0,"count":1},{"bound":2E0,"count":2},{"bound":5E0,"count":2},{"bound":1e99999,"count":2}]}
{"name":"histogram2","labels":{"more":"labels"},"type":"histogram","sum":7E0,"count":2,"buckets":[{"bound":1E0,"count":0},{"bound":2E0,"count":0},{"bound":5E0,"count":2},{"bound":1e99999,"count":2}]}
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
