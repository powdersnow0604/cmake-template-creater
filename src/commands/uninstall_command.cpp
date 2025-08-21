#include "commands.h"
#include "file_utils.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace ctc {
    namespace commands {
        
        int uninstall_command(const std::vector<std::string>& args) {
            if (args.empty()) {
                std::cerr << "Error: Argument required\n";
                std::cerr << "Usage: ctc uninstall <package-name>\n";
                std::cerr << "       ctc uninstall -L <library-path>\n";
                std::cerr << "       ctc uninstall -l <library-name>\n";
                std::cerr << "       ctc uninstall -I <include-path>\n";
                return 1;
            }
            
            const std::filesystem::path libname_path = ".libname";
            
            // Check if .libname exists
            if (!std::filesystem::exists(libname_path)) {
                std::cerr << "Error: .libname file not found. Run 'ctc init' first.\n";
                return 1;
            }
            
            try {
                utils::DependencyEntry entry;
                
                // Parse arguments
                if (args[0] == "-L") {
                    if (args.size() < 2) {
                        std::cerr << "Error: Library path required after -L\n";
                        return 1;
                    }
                    entry.type = utils::DependencyEntry::LIBRARY_PATH;
                    entry.value = args[1];
                } else if (args[0] == "-l") {
                    if (args.size() < 2) {
                        std::cerr << "Error: Library name required after -l\n";
                        return 1;
                    }
                    entry.type = utils::DependencyEntry::LIBRARY_NAME;
                    entry.value = args[1];
                } else if (args[0] == "-I") {
                    if (args.size() < 2) {
                        std::cerr << "Error: Include path required after -I\n";
                        return 1;
                    }
                    entry.type = utils::DependencyEntry::INCLUDE_PATH;
                    entry.value = args[1];
                } else if (args[0] == "-T") {
                    if (args.size() < 2) {
                        std::cerr << "Error: Toolchain file path required after -T\n";
                        return 1;
                    }
                    entry.type = utils::DependencyEntry::TOOLCHAIN_FILE;
                    entry.value = args[1];
                } else if (args[0] == "-A") {
                    if (args.size() < 2) {
                        std::cerr << "Error: Mapping required after -A. Example: -A glfw3=glfw or -A Qt6:Gui=Qt6::Gui\n";
                        return 1;
                    }
                    entry.type = utils::DependencyEntry::LINK_OVERRIDE;
                    entry.value = args[1];
                } else {
                    // Default: package or package with components
                    std::string package_name = args[0];
                    std::vector<std::string> components;
                    
                    // If provided as pkg:component, split and seed component list
                    auto sep = package_name.find(':');
                    if (sep != std::string::npos) {
                        std::string pkg = package_name.substr(0, sep);
                        std::string comp = package_name.substr(sep + 1);
                        package_name = pkg;
                        if (!comp.empty()) components.push_back(comp);
                    }
                    
                    // Parse additional components via -c flags (repeatable)
                    for (size_t i = 1; i < args.size(); ++i) {
                        if (args[i] == "-c" && i + 1 < args.size()) {
                            components.push_back(args[i + 1]);
                            ++i;
                        }
                    }
                    
                    if (!components.empty()) {
                        // Remove each component entry
                        for (const auto& comp : components) {
                            utils::DependencyEntry comp_entry;
                            comp_entry.type = utils::DependencyEntry::PACKAGE_COMPONENT;
                            comp_entry.value = package_name + ":" + comp;
                            if (!utils::remove_dependency(libname_path, comp_entry)) {
                                std::cerr << "Failed to update .libname file\n";
                                return 1;
                            }
                            std::cout << "Successfully removed package component '" << comp_entry.value << "' from .libname\n";
                        }
                    } else {
                        // Remove plain package entry
                        utils::DependencyEntry pkg_entry;
                        pkg_entry.type = utils::DependencyEntry::PACKAGE;
                        pkg_entry.value = package_name;
                        if (!utils::remove_dependency(libname_path, pkg_entry)) {
                            std::cerr << "Failed to update .libname file\n";
                            return 1;
                        }
                        std::cout << "Successfully removed package '" << pkg_entry.value << "' from .libname\n";
                    }
                    
                    std::cout << "Note: Use 'ctc run' to automatically update your CMakeLists.txt.\n";
                    return 0;
                }
                
                // Remove the dependency (non-package flags path)
                if (!utils::remove_dependency(libname_path, entry)) {
                    std::cerr << "Failed to update .libname file\n";
                    return 1;
                }
                
                std::string type_name;
                switch (entry.type) {
                    case utils::DependencyEntry::PACKAGE:
                        type_name = "package";
                        break;
                    case utils::DependencyEntry::LIBRARY_PATH:
                        type_name = "library path";
                        break;
                    case utils::DependencyEntry::LIBRARY_NAME:
                        type_name = "library";
                        break;
                    case utils::DependencyEntry::INCLUDE_PATH:
                        type_name = "include path";
                        break;
                    case utils::DependencyEntry::PACKAGE_COMPONENT:
                        type_name = "package component";
                        break;
                    case utils::DependencyEntry::TOOLCHAIN_FILE:
                        type_name = "toolchain file";
                        break;
                    case utils::DependencyEntry::LINK_OVERRIDE:
                        type_name = "link override";
                        break;
                }
                
                std::cout << "Successfully removed " << type_name << " '" << entry.value << "' from .libname\n";
                std::cout << "Note: Use 'ctc run' to automatically update your CMakeLists.txt.\n";
                
                return 0;
                
            } catch (const std::exception& e) {
                std::cerr << "Error uninstalling dependency: " << e.what() << "\n";
                return 1;
            }
        }
    }
}
