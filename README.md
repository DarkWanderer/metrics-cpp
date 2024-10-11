# metrics-cpp

[![Build](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml)
![Readiness](https://img.shields.io/badge/readiness-beta-yellow)

A low-footprint, high-performance C++ metrics library implementing commonly used metric classes - Counter, Gauge, Histogram, Summary - in idiomatic and thread-safe faction

## Design goals

The design goals of this library are the following:

* Be as lightweight as possible - all operations on Counter, Gauge, Histogram are lock-free using atomic operations
* Allow to think of instrumenting first and exposition later
* Provide easy to use API

## Key features

* Provides commonly used metric classes
* A number of out-the-box optimizations
  * all metrics except Summary are lock-free
  * Labels are optimized for cache locality (vector instead of std::map; make sure to use a compiler which takes advantage of [SSO](https://pvs-studio.com/en/blog/terms/6658/))
  * Minimized locking for operations in Registry
* Various methods of serialization
  * Prometheus
  * JSON/JSONL
  * statsd
* Cross-platform (built for Windows, Ubuntu, MacOS)

## Limitations & compromises

* Due to limited number of locks employed, there is no strong consistency guarantee between different metrics
* If a particular thread changes two counters and serialization happens in the middle, you may see a value for one counter increasing but not for the other - until the next time metrics are collected. Hence, care must be taken when creating alerts based on metrics differential
* For same reason, histogram 'sum' may be out of sync with total count - skewing the average value with ⅟n asymptotic upper bound
* It's not possible to _remove_ metrics from a `Registry` - conceptually shared with Prometheus
* Boost::accumulators do not correctly work under MacOS, which prevents Summary class from working there - more throrough investigation pending

## Readiness

|Feature|Readiness|
|----|----|
|Core API|![GA](https://img.shields.io/badge/GA-green)|
|Serialization: JSON|![icon](https://img.shields.io/badge/GA-green)|
|Serialization: Prometheus|![icon](https://img.shields.io/badge/GA-green)|
|Sink: Statsd UDP|![icon](https://img.shields.io/badge/beta-yellow)|
|Sink: Statsd TCP|![icon](https://img.shields.io/badge/beta-yellow)|
|Sink: PushGateway|![icon](https://img.shields.io/badge/disabled-red)|
|Sink: Prometheus HTTP|![icon](https://img.shields.io/badge/beta-yellow)|

## Performance

Performance of metrics is comparable to `atomic` primitives - even with pointer indirection

```
Run on (24 X 3700 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x12)
  L1 Instruction 32 KiB (x12)
  L2 Unified 512 KiB (x12)
  L3 Unified 32768 KiB (x2)
-----------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations
-----------------------------------------------------------------------
BM_Reference_AtomicIncrement       1.50 ns         1.51 ns    497777778
BM_CounterIncrement                1.34 ns         1.34 ns    560000000
BM_GaugeSet                        1.84 ns         1.84 ns    407272727
BM_Histogram2Observe               4.24 ns         4.20 ns    160000000
BM_Histogram5Observe               4.70 ns         4.60 ns    149333333
BM_Histogram10Observe              5.37 ns         5.47 ns    100000000
BM_SummaryObserve                  9.13 ns         9.21 ns     74666667
BM_RegistryGet                     39.2 ns         39.2 ns     17920000
BM_RegistryGetLabels                138 ns          138 ns      4977778
```

## Usage examples

### Quickstart

The library API was designed to provide a low barrier for entry:

```cpp
    auto metrics = createRegistry();
    metrics->getCounter( "birds", {{ "kind", "pigeon" }} )++;
    metrics->getCounter( "birds", {{ "kind", "sparrow" }} )+=10;
    metrics->getGauge( "tiredness" ) += 1.5;
    
    cout << "The library supports outputting metrics in Prometheus format:" << endl << serializePrometheus(*metrics) << endl;
    cout << "And in JSON format:" << endl << serializeJsonl(*metrics) << endl;
```

For further information on using library via CMake, see [this sample](https://github.com/DarkWanderer/metrics-cpp/tree/main/samples/cmake)

### Standalone metrics

```cpp
Counter c1;
auto c2 = c1; // Another object shares same underlying metric
c2++;
cout << c1.value(); // 1
```

### Working with registry

`Registry` is a class representing grouping of metrics within the application. Usually you would have a single `registry` per application or application domain. You can create metrics from within the registry:

```cpp
auto registry = createRegistry();
auto gauge = registry->getGauge("my_gauge", {{"some", "label"}});
gauge = 10.0;
```

Or add existing metrics with a key:

```cpp
Gauge gauge;
gauge = 5;
auto registry = createRegistry();
registry->add("my_gauge", {{"some", "label"}}, gauge);
cout << registry->getGauge("my_gauge", {{"some", "label"}}).value(); // 5
```

The recommended pattern is to instrument low-level code using standalone metrics and then add the needed metrics to a `registry` instance - this way, you can track same metrics under different names in different contexts

### Serialization

```cpp
auto registry = createRegistry();
auto gauge = registry->getGauge("my_gauge", {{"some", "label"}});
auto p = serializePrometheus(registry);
auto j = serializeJson(registry);
auto s = serializeStatsd(registry);
```

### Timers

```cpp
Histogram histogram({1., 2., 5., 10.});
for (auto file: files)
{
    Timer<std::chrono::seconds> timer(histogram); // Adds an observation to the histogram on scope exit
    process_file(file);
}
```

### Sinks

Sinks can be created explicitly by type or from a URL. In latter case, specific sink type is derived from URL schema, e.g. `statsd+udp` or `pushgateway+http`

```cpp
// Set this value in config
std::string url = "statsd+udp://localhost:1234"; 

auto registry = createRegistry();
auto gauge = registry->getGauge("my_gauge", {{"some", "label"}});
gauge = 5.0;
auto sink = createOnDemandSink(url);
if (sink)
    sink->send(registry);
```
