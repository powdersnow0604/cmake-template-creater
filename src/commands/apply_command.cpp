#include "commands.h"
#include "file_utils.h"
#include <iostream>
#include <filesystem>

namespace ctc {
    namespace commands {
        
        int apply_command(const std::vector<std::string>& args) {
            std::string project_name = "my_project"; // Default name
            
            // Parse arguments for -n flag
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] == "-n" && i + 1 < args.size()) {
                    project_name = args[i + 1];
                    break;
                }
            }
            
            const std::filesystem::path libname_path = ".libname";
            const std::filesystem::path cmake_path = "CMakeLists.txt";
            
            std::cout << "Applying dependencies to CMakeLists.txt...\n";
            
            try {
                // Check if .libname exists
                if (!std::filesystem::exists(libname_path)) {
                    std::cerr << "Error: .libname file not found. Run 'ctc init' first.\n";
                    return 1;
                }
                
                // Read dependencies from .libname
                std::cout << "Reading dependencies from .libname...\n";
                std::vector<utils::DependencyEntry> dependencies = utils::read_libname(libname_path);
                
                // Show what dependencies will be applied
                if (dependencies.empty()) {
                    std::cout << "No dependencies found in .libname file.\n";
                    std::cout << "Use 'ctc install' to add dependencies first.\n";
                } else {
                    std::cout << "Found " << dependencies.size() << " dependencies to apply.\n";
                }
                
                // Update CMakeLists.txt
                std::cout << "Updating CMakeLists.txt with project name '" << project_name << "'...\n";
                if (!utils::update_cmake_file(cmake_path, project_name, dependencies)) {
                    std::cerr << "Failed to update CMakeLists.txt\n";
                    return 1;
                }
                
                std::cout << "Successfully updated CMakeLists.txt!\n";
                
                if (!dependencies.empty()) {
                    std::cout << "\nApplied dependencies:\n";
                    
                    // Group and show dependencies
                    std::vector<utils::DependencyEntry> packages, lib_paths, lib_names, inc_paths, toolchain_paths, pkg_components, link_overrides;
                    
                    for (const auto& dep : dependencies) {
                        switch (dep.type) {
                            case utils::DependencyEntry::PACKAGE:
                                packages.push_back(dep);
                                break;
                            case utils::DependencyEntry::LIBRARY_PATH:
                                lib_paths.push_back(dep);
                                break;
                            case utils::DependencyEntry::LIBRARY_NAME:
                                lib_names.push_back(dep);
                                break;
                            case utils::DependencyEntry::INCLUDE_PATH:
                                inc_paths.push_back(dep);
                                break;
                            case utils::DependencyEntry::TOOLCHAIN_FILE:
                                toolchain_paths.push_back(dep);
                                break;
                            case utils::DependencyEntry::PACKAGE_COMPONENT:
                                pkg_components.push_back(dep);
                                break;
                            case utils::DependencyEntry::LINK_OVERRIDE:
                                link_overrides.push_back(dep);
                                break;
                        }
                    }
                    
                    if (!packages.empty()) {
                        std::cout << "  - Packages: ";
                        for (size_t i = 0; i < packages.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << packages[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!lib_paths.empty()) {
                        std::cout << "  - Library paths: ";
                        for (size_t i = 0; i < lib_paths.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << lib_paths[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!lib_names.empty()) {
                        std::cout << "  - Libraries: ";
                        for (size_t i = 0; i < lib_names.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << lib_names[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!inc_paths.empty()) {
                        std::cout << "  - Include paths: ";
                        for (size_t i = 0; i < inc_paths.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << inc_paths[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!toolchain_paths.empty()) {
                        std::cout << "  - Toolchain file: ";
                        for (size_t i = 0; i < toolchain_paths.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << toolchain_paths[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!pkg_components.empty()) {
                        std::cout << "  - Package components: ";
                        for (size_t i = 0; i < pkg_components.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << pkg_components[i].value;
                        }
                        std::cout << "\n";
                    }
                    
                    if (!link_overrides.empty()) {
                        std::cout << "  - Link overrides: ";
                        for (size_t i = 0; i < link_overrides.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << link_overrides[i].value;
                        }
                        std::cout << "\n";
                    }
                }
                
                std::cout << "\nCMakeLists.txt is now ready. Use 'ctc run' to build your project.\n";
                
                return 0;
                
            } catch (const std::exception& e) {
                std::cerr << "Error applying dependencies: " << e.what() << "\n";
                return 1;
            }
        }
    }
}
