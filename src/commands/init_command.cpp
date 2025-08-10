#include "commands.h"
#include "file_utils.h"
#include <iostream>
#include <filesystem>

namespace ctc {
    namespace commands {
        
        int init_command(const std::vector<std::string>& args) {
            bool create_readme = false;
            
            // Check for -r flag
            for (const auto& arg : args) {
                if (arg == "-r") {
                    create_readme = true;
                    break;
                }
            }
            
            std::cout << "Initializing C++ project structure...\n";
            
            try {
                // Create directories
                std::vector<std::string> directories = {"bin", "app", "lib", "include"};
                for (const auto& dir : directories) {
                    if (!utils::create_directory_if_not_exists(dir)) {
                        std::cerr << "Failed to create directory: " << dir << "\n";
                        return 1;
                    }
                    std::cout << "Created directory: " << dir << "\n";
                }
                
                // Create CMakeLists.txt
                if (!utils::write_file("CMakeLists.txt", utils::get_cmake_template())) {
                    std::cerr << "Failed to create CMakeLists.txt\n";
                    return 1;
                }
                std::cout << "Created file: CMakeLists.txt\n";
                
                // Create .gitignore
                if (!utils::write_file(".gitignore", utils::get_gitignore_template())) {
                    std::cerr << "Failed to create .gitignore\n";
                    return 1;
                }
                std::cout << "Created file: .gitignore\n";
                
                // Create .libname (empty initially)
                if (!utils::write_file(".libname", "")) {
                    std::cerr << "Failed to create .libname\n";
                    return 1;
                }
                std::cout << "Created file: .libname\n";
                
                // Create README.md if -r flag was used
                if (create_readme) {
                    if (!utils::write_file("README.md", utils::get_readme_template())) {
                        std::cerr << "Failed to create README.md\n";
                        return 1;
                    }
                    std::cout << "Created file: README.md\n";
                }
                
                std::cout << "Project initialization complete!\n";
                std::cout << "You can now add your source files to the app/ and lib/ directories.\n";
                std::cout << "Use 'ctc run' to build your project.\n";
                
                return 0;
                
            } catch (const std::exception& e) {
                std::cerr << "Error during initialization: " << e.what() << "\n";
                return 1;
            }
        }
    }
}
