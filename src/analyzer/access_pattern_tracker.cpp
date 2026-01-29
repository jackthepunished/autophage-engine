#include <autophage/analyzer/access_pattern_tracker.hpp>

namespace autophage::analyzer {

void AccessPatternTracker::recordAccess(const std::string& systemName, AccessPattern pattern,
                                        u32 count)
{
    stats_[systemName] = {systemName, pattern, count};
}

const std::unordered_map<std::string, SystemStats>& AccessPatternTracker::getStats() const
{
    return stats_;
}

void AccessPatternTracker::reset()
{
    stats_.clear();
}

}  // namespace autophage::analyzer
