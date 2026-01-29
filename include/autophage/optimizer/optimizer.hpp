#pragma once

#include <autophage/analyzer/stats_collector.hpp>
#include <autophage/core/types.hpp>
#include <autophage/ecs/world.hpp>


namespace autophage::optimizer {

class Optimizer
{
public:
    explicit Optimizer(analyzer::StatsCollector& stats);

    void update(ecs::World& world);

private:
    analyzer::StatsCollector& stats_;
};

}  // namespace autophage::optimizer
