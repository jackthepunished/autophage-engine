#include <autophage/core/logger.hpp>
#include <autophage/rewriter/jit_compiler.hpp>

#ifdef AUTOPHAGE_JIT_ENABLED
    #include <llvm/ExecutionEngine/Orc/LLJIT.h>
    #include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
    #include <llvm/IR/IRBuilder.h>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/Module.h>
    #include <llvm/Support/TargetSelect.h>
    #include <llvm/Support/raw_ostream.h>
#endif

namespace autophage::rewriter {

class JITCompiler::Impl
{
public:
    Impl()
    {
#ifdef AUTOPHAGE_JIT_ENABLED
        LOG_INFO("Initializing LLVM JIT components...");
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        auto jitOrErr = llvm::orc::LLJITBuilder().create();
        if (auto err = jitOrErr.takeError()) {
            lastError_ = llvm::toString(std::move(err));
            LOG_ERROR("Failed to create LLJIT: {}", lastError_);
            return;
        }

        jit_ = std::move(*jitOrErr);
        LOG_INFO("LLVM JIT (OrcJIT v2) initialized successfully.");
#endif
    }

    ~Impl() = default;

#ifdef AUTOPHAGE_JIT_ENABLED
    std::unique_ptr<llvm::orc::LLJIT> jit_;
#endif
    std::string lastError_;
};

JITCompiler::JITCompiler() : impl_(std::make_unique<Impl>()) {}
JITCompiler::~JITCompiler() = default;

void* JITCompiler::compile(const std::string& source, const std::string& functionName)
{
#ifdef AUTOPHAGE_JIT_ENABLED
    if (!impl_->jit_) {
        impl_->lastError_ = "JIT not initialized";
        return nullptr;
    }

    // For now, this is a placeholder for actual source compilation.
    // Real implementation would use Clang to generate IR or build IR manually.
    LOG_WARN("Source compilation not yet implemented. Use IR compilation or assembly.");
    impl_->lastError_ = "Source compilation (C++ string) not yet implemented";
    return nullptr;
#else
    (void)source;
    (void)functionName;
    return nullptr;
#endif
}

void JITCompiler::addSymbol(const std::string& name, void* address)
{
#ifdef AUTOPHAGE_JIT_ENABLED
    if (!impl_->jit_)
        return;

    auto& mainJitDylib = impl_->jit_->getMainJITDylib();
    llvm::orc::SymbolMap symbols;
    symbols[impl_->jit_->mangleAndIntern(name)] = {llvm::orc::ExecutorAddr::fromPtr(address),
                                                   llvm::JITSymbolFlags::Exported};

    if (auto err = mainJitDylib.define(llvm::orc::absoluteSymbols(symbols))) {
        impl_->lastError_ = llvm::toString(std::move(err));
        LOG_ERROR("Failed to add symbol '{}': {}", name, impl_->lastError_);
    }
#else
    (void)name;
    (void)address;
#endif
}

std::string JITCompiler::getLastError() const
{
    return impl_->lastError_;
}

bool JITCompiler::isAvailable() const noexcept
{
#ifdef AUTOPHAGE_JIT_ENABLED
    return true;
#else
    return false;
#endif
}

}  // namespace autophage::rewriter
