# metrics-cpp

[![Build](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/DarkWanderer/metrics-cpp/actions/workflows/build.yml)

A low-footprint, high-performance C++ metrics library implementing commonly used metric classes - Counter, Gauge, Histogram, Summary - in idiomatic and thread-safe faction

Key features:

* Commonly used metric classes
  * all metrics except Summary are lock-free
* Prometheus, Json, Jsonl serialization
* Cross-platform (built for Windows, Ubuntu, MacOS)

## Usage examples

### Quickstart

```cpp
defaultRegistry().getCounter( { "birds", {{ "kind", "sparrow" }} } )++;
defaultRegistry().getGauge( { "tiredness" } ) += 5;
std::cout << serializePrometheus(defaultRegistry()) << std::endl;
```

### Standalone metrics

```cpp
Counter c1;
auto c2 = c1; // Another object shares same underlying metric
c2++;
cout << c1.value(); // 1
```

### Registering an existing metric

`Registry`

```cpp
auto registry = createRegistry();
auto gauge = registry->getGauge("my_gauge", {{"some", "label"}});
gauge = 10.0;
```
Registry also allows adding previously existing metrics:
```cpp
Gauge gauge;
gauge = 5;
auto registry = createRegistry();
registry->add("my_gauge", {{"some", "label"}}, gauge);
cout << registry->getGauge("my_gauge", {{"some", "label"}}).value(); // 5
```

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
