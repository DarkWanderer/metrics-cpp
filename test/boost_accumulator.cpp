#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace boost::accumulators;

// This is a sanity check designed to validate that 
// Boost.Accumulators library is correctly linked and working
TEST_CASE("Boost.Accumulators", "[boost][accumulator]")
{
    accumulator_set<double, stats<tag::count, tag::sum, tag::tail_quantile<right>>> accumulator;
    for (int i = 1; i <= 100; i++)
        accumulator((double)i);
    CHECK(boost::accumulators::count(accumulator) == 100);
    CHECK(boost::accumulators::sum(accumulator) == 5050);
}
