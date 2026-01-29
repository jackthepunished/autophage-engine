#include <autophage/core/logger.hpp>
#include <autophage/rewriter/hot_swap_manager.hpp>
#include <autophage/rewriter/rewriter.hpp>

namespace autophage::rewriter {

HotSwapManager::HotSwapManager(ecs::World& world)
    : world_(world), compiler_(std::make_unique<JITCompiler>())
{}

HotSwapManager::~HotSwapManager() = default;

bool HotSwapManager::hotSwapFromSource(const std::string& systemName, const std::string& source)
{
    LOG_INFO("Attempting to hot-swap system '{}' from source...", systemName);

    if (!compiler_->isAvailable()) {
        LOG_ERROR("JIT Compiler is not available. Cannot hot-swap from source.");
        return false;
    }

    // 1. Map necessary engine symbols (World, types, etc.)
    compiler_->addSymbol("world", &world_);

    // 2. Compile the source
    void* funcPtr = compiler_->compile(source, "updateSystem");
    if (!funcPtr) {
        LOG_ERROR("Failed to compile system source: {}", compiler_->getLastError());
        return false;
    }

    // 3. Transform the function pointer to a JITSystem UpdateFunc
    auto updateFunc = reinterpret_cast<JITSystem::UpdateFunc>(funcPtr);

    // 4. Replace the system in the world
    // We replace the ISystem but pinpoint it via the systemName if we had a registry lookup by
    // name. For now, we assume we are replacing the specific system by type if it was registered.
    world_.replaceSystem<ecs::ISystem, JITSystem>(updateFunc);

    LOG_INFO("Successfully hot-swapped system '{}' with JIT'd implementation.", systemName);

    return true;
}

}  // namespace autophage::rewriter
