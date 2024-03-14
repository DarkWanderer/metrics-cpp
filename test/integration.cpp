#include <metrics/registry.h>
#include <metrics/sink.h>

#include <iostream>

using namespace Metrics;
using namespace std;

std::shared_ptr<IRegistry> createTestRegistry() {
    auto registry = createRegistry();
    registry->getCounter("counter1") += 1;
    registry->getCounter("counter2", { { "label", "value1" } }) += 1;
    registry->getCounter("counter2", { { "label", "value2" } }) += 2;
    registry->getGauge("gauge1") = 100.;
    registry->getGauge("gauge2", { { "another", "label" } }) = 200.;

    registry->setDescription("counter1", "Description of counter 1");

    registry->getHistogram("histogram1", {}, { 1., 2., 5. }).observe(1).observe(2);
    registry->getHistogram("histogram2", { {"more", "labels"} }, { 1., 2., 5. }).observe(3).observe(4);

    registry->getSummary("summary1").observe(1).observe(2).observe(3);
    registry->getSummary("summary2", { {"summary", "label"} }).observe(3).observe(3).observe(5);
    return registry;
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2)
            throw logic_error("Need exactly 1 argument (url)");

        auto url = argv[1];
        auto registry = createTestRegistry();
        auto sink = createOnDemandSink(url);
        if (sink)
            sink->send(registry);
        else
            throw logic_error("Could not create sink for url");
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what();
        return 1;
    }
    return 0;
}
