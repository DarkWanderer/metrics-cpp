#include <benchmark/benchmark.h>
#include <metrics/registry.h>

#include <atomic>
#include <thread>

using namespace Metrics;

static const size_t maxThreads = std::thread::hardware_concurrency();

static void BM_Reference_AtomicIncrement(benchmark::State& state) {
    static std::atomic<int> counter;
    for (auto _ : state)
        counter++;
    state.SetItemsProcessed(counter.load());
}
BENCHMARK(BM_Reference_AtomicIncrement)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_CounterIncrement(benchmark::State& state) {
    static auto counter = Counter();
    for (auto _ : state)
        counter++;
    state.SetItemsProcessed(counter.value());
}
BENCHMARK(BM_CounterIncrement)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_GaugeSet(benchmark::State& state) {
    static auto gauge = Gauge();
    for (auto _ : state)
        gauge = 1;
}
BENCHMARK(BM_GaugeSet)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_GaugeIncrement(benchmark::State& state) {
    static auto gauge = Gauge();
    for (auto _ : state)
        gauge += 1;
    state.SetItemsProcessed(gauge.value());
}
BENCHMARK(BM_GaugeIncrement)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_HistogramObserve(benchmark::State& state) {
    static std::shared_ptr<IHistogram> histogram;

    if (state.thread_index() == 0) {
        int nBuckets = state.range(0);
        std::vector<double> buckets;
        buckets.reserve(nBuckets);

        for (int i = 0; i < nBuckets; i++)
            buckets.push_back(i+1);

        histogram = makeHistogram(buckets);
    }

    for (auto _ : state)
        histogram->observe(5);

    state.SetItemsProcessed(histogram->count());
}
BENCHMARK(BM_HistogramObserve)->Range(2, 5)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_SummaryObserve(benchmark::State& state) {
    static auto summary = makeSummary({ 0.9, 0.99, 0.999 }, 0.01);
    for (auto _ : state)
        summary->observe(5);
    state.SetItemsProcessed(summary->count());
}
BENCHMARK(BM_SummaryObserve)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_RegistryGet(benchmark::State& state) {
    static auto registry = createRegistry();
    registry->getCounter("test");
    for (auto _ : state)
        registry->getCounter("test");
}
BENCHMARK(BM_RegistryGet)->ThreadRange(1, maxThreads)->UseRealTime();

static void BM_RegistryGetLabels(benchmark::State& state) {
    static auto registry = createRegistry();
    registry->getCounter("test", { {"a", "b"} });
    for (auto _ : state)
        registry->getCounter("test", { {"a", "b"} });
}
BENCHMARK(BM_RegistryGetLabels)->ThreadRange(1, maxThreads)->UseRealTime();

BENCHMARK_MAIN();
