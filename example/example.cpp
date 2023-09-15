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
    int randNum = (rand() % 3) + 1;
    sleep(randNum);
}

void metricsTest() {
    Metrics::defaultRegistry().getCounter({"birds", {{"kind", "sparrow" }}})++;
    Metrics::defaultRegistry().getCounter({"birds", {{"kind", "robin" }}})++;
    Metrics::defaultRegistry().getCounter({"helloworld"})++;
    Metrics::defaultRegistry().getCounter({"helloworld"})++;

    Metrics::Gauge g = Metrics::defaultRegistry().getGauge({"foo"});
    g = 378924;
    g = 111256;
    g += 3;
    
    timerExample();
    timerExample();

    Metrics::serializeJSON(Metrics::defaultRegistry(), "example.json");
}

int main() {
    srand(time(NULL));
    std::cout << "Hello World!" << std::endl;
    metricsTest();
    std::cout << "Goodbye World!" << std::endl;
    return 0;
}