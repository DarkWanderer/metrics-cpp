# metrics-cpp

## Usage examples

### Quickstart

```cpp
defaultRegistry().getCounter( { "birds", {{ "kind", "sparrow" }} } )++;
defaultRegistry().getGauge( { "tiredness" } ) += 5;
```

### Standalone metrics

```cpp
Counter c1;
auto c2 = c1; // Another object shares same underlying metric
c2++;
cout << c1.value(); // 1
```

### Registering an existing metric

```cpp
Gauge gauge;
gauge = 5;
auto registry = createRegistry();
registry->add({ "my_gauge", {{"some", "label"}} }, gauge);
cout << registry->getGauge({ "my_gauge", {{"some", "label"}} }).value(); // 5
```

### Prometheus

```cpp
auto registry = createRegistry();
auto gauge = registry->getGauge({ "my_gauge", {{"some", "label"}} });
auto result = serializePrometheus(*registry);
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
