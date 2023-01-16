#pragma once

#include <map>
#include <string>

namespace Metrics
{
    typedef std::pair<std::string, std::string> Label;
    // TODO: use vector-based container
    typedef std::map<std::string, std::string> Labels;
}
