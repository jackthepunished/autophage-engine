#pragma once

#include <autophage/core/types.hpp>
#include <autophage/profiler/profiler.hpp>

#include <string>
#include <vector>

namespace autophage::analyzer {

struct OptimizationHint
{
    std::string subsystem;
    std::string message;
    u8 severity;  // 0=Info, 1=Warning, 2=Critical
};

class StatsCollector
{
public:
    void collect();
    [[nodiscard]] std::vector<OptimizationHint> analyze() const;

private:
    ProfilerStats currentStub_;
};

}  // namespace autophage::analyzer
