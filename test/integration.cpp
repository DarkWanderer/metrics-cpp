#include <metrics/registry.h>
#include <metrics/statsd.h>
#include <metrics/prometheus.h>

#include <iostream>

using namespace Metrics;
using namespace std;

int main(int argc, char* argv[]) {
    try {
        if (argc != 3)
            throw logic_error("Wrong number of arguments");

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

        string sink_name = argv[1];
        uint16_t port = std::stoi(argv[2]);

        shared_ptr<IOnDemandSink> sink;
        if (sink_name == "statsd_tcp") {
            sink = Statsd::createTcpSink("localhost", port);
        }
        else if (sink_name == "statsd_udp") {
            sink = Statsd::createUdpSink("localhost", port);
        }
        else if (sink_name == "pushgateway") {
            sink = Prometheus::createPushGatewaySink("localhost", port);
        }
        if (sink)
            sink->send(registry);
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what();
        return 666;
    }
    return 0;
}
