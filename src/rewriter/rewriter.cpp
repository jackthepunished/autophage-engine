#include <autophage/rewriter/rewriter.hpp>

#include <sstream>


namespace autophage::rewriter {

std::string Rewriter::generateSystemSource(const std::string& name,
                                           const std::vector<std::string>& components,
                                           const std::string& logic)
{
    std::stringstream ss;

    // Includes
    ss << "#include <autophage/ecs/world.hpp>\n";
    ss << "#include <autophage/ecs/components.hpp>\n\n";

    // Function signature
    ss << "extern \"C\" void " << name << "(autophage::ecs::World& world, float dt) {\n";

    // Query setup
    ss << "    auto q = world.query<";
    for (size_t i = 0; i < components.size(); ++i) {
        ss << components[i] << (i == components.size() - 1 ? "" : ", ");
    }
    ss << ">();\n\n";

    // Loop
    ss << "    q.forEach([dt](auto entity, ";
    for (size_t i = 0; i < components.size(); ++i) {
        ss << "auto& comp" << i << (i == components.size() - 1 ? "" : ", ");
    }
    ss << ") {\n";

    // User logic
    ss << "        " << logic << "\n";

    ss << "    });\n";
    ss << "}\n";

    return ss.str();
}

std::string Rewriter::generateSystemClass(const std::string& className, const std::string& logic)
{
    std::stringstream ss;

    ss << "#include <autophage/ecs/system.hpp>\n";
    ss << "#include <autophage/ecs/world.hpp>\n\n";

    ss << "class " << className << " : public autophage::ecs::System<" << className << "> {\n";
    ss << "public:\n";
    ss << "    " << className << "() : System(\"" << className << "\") {}\n\n";
    ss << "    void update(autophage::ecs::World& world, float dt) override {\n";
    ss << "        " << logic << "\n";
    ss << "    }\n";
    ss << "};\n\n";

    // Factory function
    ss << "extern \"C\" autophage::ecs::ISystem* createSystem() {\n";
    ss << "    return new " << className << "();\n";
    ss << "}\n";

    return ss.str();
}

}  // namespace autophage::rewriter
