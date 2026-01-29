#pragma once

#include <autophage/core/types.hpp>

#include <string>
#include <unordered_map>

namespace autophage::analyzer {

enum class AccessPattern : u8
{
    Linear,  // Iterating views (cache-friendly)
    Random,  // Random entity access (cache-unfriendly)
    Mixed    // Both
};

struct SystemStats
{
    std::string systemName;
    AccessPattern pattern = AccessPattern::Linear;
    u32 processedEntities = 0;
};

class AccessPatternTracker
{
public:
    void recordAccess(const std::string& systemName, AccessPattern pattern, u32 count);
    [[nodiscard]] const std::unordered_map<std::string, SystemStats>& getStats() const;
    void reset();

private:
    std::unordered_map<std::string, SystemStats> stats_;
};

}  // namespace autophage::analyzer
