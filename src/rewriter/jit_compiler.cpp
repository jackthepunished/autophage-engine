#include <autophage/core/logger.hpp>
#include <autophage/rewriter/jit_compiler.hpp>


namespace autophage::rewriter {

class JITCompiler::Impl
{
public:
    Impl()
    {
#ifdef AUTOPHAGE_JIT_ENABLED
        LOG_INFO("Initializing LLVM JIT...");
        // LLVM initialization logic would go here
#endif
    }

    ~Impl()
    {
#ifdef AUTOPHAGE_JIT_ENABLED
        LOG_INFO("Shutting down LLVM JIT...");
#endif
    }
};

JITCompiler::JITCompiler() : impl_(std::make_unique<Impl>()) {}
JITCompiler::~JITCompiler() = default;

bool JITCompiler::isAvailable() const noexcept
{
#ifdef AUTOPHAGE_JIT_ENABLED
    return true;
#else
    return false;
#endif
}

}  // namespace autophage::rewriter
