#include <autophage/analyzer/stats_collector.hpp>

namespace autophage::analyzer {

void StatsCollector::collect()
{
    // In a real implementation, we would store historical data or fetch from Profiler.
    // access autophage::getProfilerStats();
    currentStub_ = autophage::getProfilerStats();
}

std::vector<OptimizationHint> StatsCollector::analyze() const
{
    std::vector<OptimizationHint> hints;

    // Example analysis
    if (currentStub_.avgFps < 30.0 && currentStub_.sampleCount > 10) {
        hints.push_back({"Engine", "Low FPS detected (< 30)", 2});
    }

    if (currentStub_.spikeCount > 5) {
        hints.push_back({"Engine", "Frame time spikes detected", 1});
    }

    return hints;
}

}  // namespace autophage::analyzer
