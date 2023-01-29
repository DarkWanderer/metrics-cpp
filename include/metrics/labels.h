#pragma once

#include <metrics/vecmap.h>
#include <string>

namespace Metrics
{
    typedef std::pair<std::string, std::string> Label;
    // TODO: use vector-based container
    typedef vecmap<std::string, std::string> Labels;
}
