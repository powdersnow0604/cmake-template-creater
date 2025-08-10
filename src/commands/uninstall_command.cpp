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
                } else {
                    // Default - assume it's a package name
                    entry.type = utils::DependencyEntry::PACKAGE;
                    entry.value = args[0];
                }
                
                // Remove the dependency
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
