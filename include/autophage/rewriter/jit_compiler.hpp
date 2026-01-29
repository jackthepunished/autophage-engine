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
    /// @return A void pointer to the function or nullptr on failure
    void* compile(const std::string& source, const std::string& functionName);

    /// @brief Map a function pointer to a symbol name in the JIT
    void addSymbol(const std::string& name, void* address);

    /// @brief Get the last error message
    [[nodiscard]] std::string getLastError() const;

    /// @brief Check if JIT is enabled and available
    [[nodiscard]] bool isAvailable() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace autophage::rewriter
