#include <benchmark/benchmark.h>
#include <metrics/registry.h>

using namespace Metrics;

static void BM_CounterIncrement(benchmark::State& state) {
    auto counter = Counter();
    for (auto _ : state)
        counter++;
}
BENCHMARK(BM_CounterIncrement);

static void BM_GaugeSet(benchmark::State& state) {
    auto gauge = Gauge();
    for (auto _ : state)
        gauge = 123;
}
BENCHMARK(BM_GaugeSet);

static void BM_HistogramObserve(benchmark::State& state) {
    auto histogram = makeHistogram({ 1,2,3,4,5 });
    for (auto _ : state)
        histogram->observe(5);
}
BENCHMARK(BM_HistogramObserve);

static void BM_SummaryObserve(benchmark::State& state) {
    auto summary = makeSummary({ 0.9, 0.99, 0.999 }, 0.01);
    for (auto _ : state)
        summary->observe(5);
}
BENCHMARK(BM_SummaryObserve);

static void BM_RegistryGet(benchmark::State& state) {
    auto registry = createRegistry();
    registry->getCounter("test");
    for (auto _ : state)
        registry->getCounter("test");
}
BENCHMARK(BM_RegistryGet);

static void BM_RegistryGetLabels(benchmark::State& state) {
    auto registry = createRegistry();
    registry->getCounter("test", { {"a", "b"} });
    for (auto _ : state)
        registry->getCounter("test", { {"a", "b"} });
}
BENCHMARK(BM_RegistryGetLabels);

BENCHMARK_MAIN();
