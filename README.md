# metrics-cpp

[![Build](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml)

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
* Planned: HTTP/UDP sinks

## Limitations & compromises

Due to limited number of locks employed, there is no strong consistency guarantee between different metrics. If a particular thread changes two counters and serialization happens in the middle, you may see a value for one counter increasing but not for the other - until the next time metrics are collected. Hence, care must be taken when creating alerts based on metrics differential

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

# 3rd-party tools and libraries

This project utilizes following 3rd-party libraries and tools

[nlohmann-json](https://github.com/nlohmann/json/) - fast C++ JSON library
[stream-quantiles](https://github.com/Liam0205/stream-quantiles) - C++ CKMS algorithm implementation
[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
