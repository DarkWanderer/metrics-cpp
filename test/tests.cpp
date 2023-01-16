#include <counter.h>
#include <gauge.h>
#include <registry.h>

int main() {
    auto& registry = Metrics::defaultRegistry();
    
    auto counter = registry.getCounter();
    counter++;
    
    auto gauge = registry.getGauge();
    gauge = 1.0;
    gauge += 0.5;
    gauge -= 1.5;
}
