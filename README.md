# metrics-cpp

## Usage examples

### Standalone metric

```cpp
Counter c1;
auto c2 = c1; // Another object shares same underlying metric
c2++;
cout << c1.value(); // 1
```

### Registry

```cpp
auto registry = createRegistry();
auto gauge = registry->getGauge({ "my_gauge", {{"some", "label"}} });
gauge = 10.0;
```
Registry also allows adding previously existing metrics:
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
