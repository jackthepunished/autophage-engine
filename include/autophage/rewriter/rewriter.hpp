#pragma once

#include <autophage/ecs/world.hpp>

#include <string>
#include <vector>


namespace autophage::rewriter {

/// @brief Generates C++ code for ECS systems
class Rewriter
{
public:
    Rewriter() = default;

    /// @brief Generate a specialized update function for a query
    /// @param name The name of the generated function
    /// @param components The list of component type names
    /// @param logic The logic string to insert into the loop
    std::string generateSystemSource(const std::string& name,
                                     const std::vector<std::string>& components,
                                     const std::string& logic);

    /// @brief Generate a full System class implementation
    std::string generateSystemClass(const std::string& className, const std::string& logic);
};

}  // namespace autophage::rewriter
