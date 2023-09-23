#include <metrics/registry.h>
#include <metrics/serialize.h>
#include <metrics/timer.h>
using namespace Metrics;

#include <iostream>
using namespace std;

int main()
{
    cout << "This is a sample app demonstrating usage of simple metrics" << endl << endl;
    auto birdwatching_metrics = createRegistry();
    birdwatching_metrics->getCounter( { "birds", {{ "kind", "pigeon" }} } )++;
    birdwatching_metrics->getCounter( { "birds", {{ "kind", "sparrow" }} } )+=10;
    birdwatching_metrics->getGauge( { "tiredness" } ) += 1.5;
    
    cout << "The library supports outputting metrics in Prometheus format:" << endl << serializePrometheus(*birdwatching_metrics) << endl;
    cout << "And in JSON format:" << endl << serializeJsonl(*birdwatching_metrics) << endl;
    return 0;
}