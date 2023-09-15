# metrics-cpp

This library aims to create a simple and lightweight metrics tracking system for C++ applications. It supports basic metrics types the developers would often desire when instrumenting their code. The library supports dumping metrics out to either JSON or Prometheus format.

The library compiles as a shared library `libmetrics.so` that is to be linked in by any project that requires instrumentation.

## Usage

See the [Example Code](https://github.com/KyleLavorato/metrics-cpp-json/tree/main/example/README.md) for a complete and executable example, including how to link in the compiled metrics shared library.

```cpp
#include <iostream>
#include <string>
#include <unistd.h>
#include <chrono>

#include <metrics/registry.h>
#include <metrics/serialize.h>
#include <metrics/timer.h>


void timerExample() {
    Metrics::Histogram histogram = Metrics::defaultRegistry().getHistogram({"myTimer"}, {1., 2., 5., 10.});
    Metrics::Timer<std::chrono::seconds> timer(histogram);
    sleep(2);
}

int main() {
    Metrics::defaultRegistry().getCounter({"birds", {{"kind", "sparrow"}}})++;
    Metrics::defaultRegistry().getCounter({"birds", {{"kind", "robin"}}})++;
    Metrics::defaultRegistry().getCounter({"helloworld"})++;

    Metrics::defaultRegistry().getGauge({"foo"}) = 2007;
    
    timerExample();

    Metrics::serializeJSON(Metrics::defaultRegistry(), "example.json");
    return 0;
}
```

### Registry

All metrics are saved to a base metric registry. Multiple registries can be created and managed to separate metrics sets, or the default registry can be easily accessed anywhere for all metrics.

```cpp
Metrics::IRegistry customRegistry = Metrics::createRegistry();
Metrics::IRegistry defaultRegistry = Metrics::defaultRegistry();
```

## Supported Metrics

The library currently supports the following metrics types:

### Counter

Counters are basic number metrics that can be incremented or decremented by one on each interaction.

```cpp
defaultRegistry().getCounter({"birds", {{"kind", "sparrow"}}})++;
```

### Gauge

Gauges are basic value metrics, where any numeric value can be written to them at any time.

```cpp
defaultRegistry().getGauge( { "tiredness" } ) += 5;
```

### Histogram

Histograms track how values change over time, and output the statistics, as well as basic values such as count, average, and sum.

```cpp
Histogram histogram = defaultRegistry().getHistogram({"histogram"}, {1., 2., 5., 10.});
```

### Timer

Timers are initialized from an existing histogram metric instead of having a dedicated type. Once a `Timer` instance is started, it will time the execution **until the current scope completes.** 

```cpp
Histogram histogram = defaultRegistry().getHistogram({"myTimer"}, {1., 2., 5., 10.});
Timer<std::chrono::seconds> timer(histogram);
```

### Standalone metrics

Metrics can be created outside the scope of a registry and added at a later time.

```cpp
Gauge gauge;
gauge = 5;
defaultRegistry()->add({ "my_gauge", {{"some", "label"}} }, gauge);
std::cout << registry->getGauge({ "my_gauge", {{"some", "label"}} }).value() << std::endl;
```

## Supported Output

The library currently supports the following output methods

### JSON File

Metrics can be output to a basic JSON format, with full support for metric family types for nested metrics. The JSON output is invoked on a registry to output the contents to a specified JSON file.

```cpp
Metrics::serializeJSON(Metrics::defaultRegistry(), "example.json");
```

```json
{
    "birds": {
        "robin": 1,
        "sparrow": 1
    },
    "foo": 2007,
    "helloworld": 2,
    "myTimer": {
        "1x": 1,
        "2x": 1,
        "5x": 2,
        "10x": 2,
        "average": 2.0,
        "count": 2,
        "sum": 4.0
    }
}
```

### Prometheus Format

If the Prometheus backend is used for managing metrics, the values can be output in the Prometheus format. This method will output the metrics into a string variable. The user is responsible for sending them to the backend on their own.

```cpp
std::string result = Metrics::serializePrometheus(Metrics::defaultRegistry());
std::cout << result << std::endl;
```

```
birds{kind="robin"} 1
birds{kind="sparrow"} 1
foo 2007
helloworld 2
# TYPE myTimer histogram
myTimer{le="1"} 1
myTimer{le="2"} 1
myTimer{le="5"} 2
myTimer{le="10"} 2
myTimer_sum 4
myTimer_count 2
```

## Requirements

Using `metrics-cpp-json` requires CMake >= 3.19 and a C++11 compliant compiler. It has been tested with GCC 9.4.0 on Ubuntu 20.04.

The [rapidjson](https://github.com/Tencent/rapidjson/) module will be dynamically fetched as part of the compile process as it is required for JSON serialization. 

## Building

The supported method to build `metrics-cpp-json` is through the use of `CMake`. The compile process is built as usual.

```bash
mkdir build
cd build

cmake ..
make
```

### Artifacts

The output artifact is `libmetrics.so`, located in the `build` directory. 

There are two sets of header files that are required to link the artifact into an application:

1. All files in the [include](https://github.com/KyleLavorato/metrics-cpp-json/tree/main/include) directory in the repository.
2. A header `build/src/metrics_export.h` that is generated at compile time based on the host machine.
