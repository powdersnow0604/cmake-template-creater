#include "commands.h"
#include "file_utils.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace ctc {
    namespace commands {
        
        int list_command(const std::vector<std::string>& args) {
            const std::filesystem::path libname_path = ".libname";
            
            // Check if .libname exists
            if (!std::filesystem::exists(libname_path)) {
                std::cerr << "Error: .libname file not found. Run 'ctc init' first.\n";
                return 1;
            }
            
            try {
                // Read dependencies from .libname
                std::vector<utils::DependencyEntry> dependencies = utils::read_libname(libname_path);
                
                if (dependencies.empty()) {
                    std::cout << "No dependencies found in .libname file.\n";
                    std::cout << "Use 'ctc install' to add dependencies.\n";
                    return 0;
                }
                
                std::cout << "Dependencies in .libname:\n";
                std::cout << std::string(50, '=') << "\n";
                
                // Group dependencies by type for better display
                std::vector<utils::DependencyEntry> packages, lib_paths, lib_names, inc_paths, toolchain_paths;
                
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
                    }
                }
                
                // Display packages
                if (!packages.empty()) {
                    std::cout << "\n[PACKAGES] CMake find_package:\n";
                    for (const auto& pkg : packages) {
                        std::cout << "  * " << pkg.value << "\n";
                    }
                }
                
                // Display library paths
                if (!lib_paths.empty()) {
                    std::cout << "\n[LIBPATHS] Library Paths (-L):\n";
                    for (const auto& path : lib_paths) {
                        std::cout << "  * " << path.value << "\n";
                    }
                }
                
                // Display libraries
                if (!lib_names.empty()) {
                    std::cout << "\n[LIBRARIES] Libraries (-l):\n";
                    for (const auto& lib : lib_names) {
                        std::cout << "  * " << lib.value << "\n";
                    }
                }
                
                // Display include paths
                if (!inc_paths.empty()) {
                    std::cout << "\n[INCLUDES] Include Paths (-I):\n";
                    for (const auto& path : inc_paths) {
                        std::cout << "  * " << path.value << "\n";
                    }
                }
                
                // Display toolchain file
                if (!toolchain_paths.empty()) {
                    std::cout << "\n[TOOLCHAIN] CMAKE_TOOLCHAIN_FILE (-T):\n";
                    for (const auto& tc : toolchain_paths) {
                        std::cout << "  * " << tc.value << "\n";
                    }
                }
                
                std::cout << "\n" << std::string(50, '=') << "\n";
                std::cout << "Total: " << dependencies.size() << " dependencies\n";
                std::cout << "\nUse 'ctc run' to automatically apply these dependencies to your CMakeLists.txt\n";
                
                return 0;
                
            } catch (const std::exception& e) {
                std::cerr << "Error reading dependencies: " << e.what() << "\n";
                return 1;
            }
        }
    }
}
