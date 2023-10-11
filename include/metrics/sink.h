#pragma once

#include <metrics_export.h>

#include <memory>
#include <string>

namespace Metrics {
    class IMetric;
    class IRegistry;

    struct IOnDemandSink {
        virtual void send(std::shared_ptr<IRegistry> registry) = 0;
        virtual ~IOnDemandSink() = 0;
    };

    struct IRegistrySink {
        virtual std::shared_ptr<IRegistry> registry() const = 0;
        virtual ~IRegistrySink() = 0;
    };
}
