#include <registry.h>

int main()
{
    auto &registry = Metrics::defaultRegistry();

    auto counter = registry.getCounter({"counter"});
    counter++;

    auto gauge = registry.getGauge({"gauge"});
    gauge = 1.0;
    gauge += 0.5;
    gauge -= 1.5;
}
