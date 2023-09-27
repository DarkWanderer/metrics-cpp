# metrics-cpp

[![Build](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml)

A low-footprint, high-performance C++ metrics library implementing commonly used metric classes - Counter, Gauge, Histogram, Summary - in idiomatic and thread-safe faction

Key features:

* Provides commonly used metric classes
* A number of out-the-box optimizations
  * all metrics except Summary are lock-free
  * Labels are optimized for cache locality (vector instead of std::map)
  * Minimized locking for operations in Registry
* Various methods of serialization
  * Prometheus
  * JSON/JSONL
  * statsd
* Cross-platform (built for Windows, Ubuntu, MacOS)

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
auto p = serializePrometheus(*registry);
auto j = serializeJson(*registry);
auto s = serializeStatsd(*registry);
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
