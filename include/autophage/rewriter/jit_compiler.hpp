#pragma once

#include <autophage/core/types.hpp>

#include <functional>
#include <memory>
#include <string>


namespace autophage::rewriter {

/// @brief Interface for runtime compilation
class JITCompiler
{
public:
    JITCompiler();
    ~JITCompiler();

    /// @brief Compile a C++ string to a function
    /// @param source The C++ source code
    /// @param functionName The name of the function to retrieve
    /// @return A function pointer or nullptr on failure
    template <typename T>
    T* compileAndGet(const std::string& source, const std::string& functionName)
    {
        (void)source;
        (void)functionName;
        // Placeholder implementation
        return nullptr;
    }

    /// @brief Check if JIT is enabled and available
    [[nodiscard]] bool isAvailable() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace autophage::rewriter
