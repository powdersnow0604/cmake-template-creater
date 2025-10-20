#include "commands.h"
#include "file_utils.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sstream>

namespace ctc {
    namespace commands {
        
        int run_command(const std::vector<std::string>& args) {
            std::string project_name = "my_project"; // Default name
            std::string build_mode = "Release"; // Default to Release mode
            bool keep_build_directory = false; // Default to cleanup after build
            bool update_cmake = false; // Default: do not update CMakeLists.txt
            
            // Parse arguments for -n and -m flags
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] == "-n" && i + 1 < args.size()) {
                    project_name = args[i + 1];
                } else if (args[i] == "-m" && i + 1 < args.size()) {
                    std::string mode = args[i + 1];
                    // Validate build mode
                    if (mode == "Debug" || mode == "Release" || mode == "MinSizeRel" || mode == "RelWithDebInfo") {
                        build_mode = mode;
                    } else {
                        std::cerr << "Error: Invalid build mode '" << mode << "'. Valid modes are: Debug, Release, MinSizeRel, RelWithDebInfo\n";
                        return 1;
                    }
                } else if (args[i] == "-k" || args[i] == "--keep-build") {
                    keep_build_directory = true;
                } else if (args[i] == "-U" || args[i] == "--update-cmake") {
                    update_cmake = true;
                }
            }
            const std::filesystem::path current_dir = std::filesystem::current_path();
            const std::filesystem::path build_dir = "build";
            const std::filesystem::path bin_dir = "bin";
            const std::filesystem::path libname_path = ".libname";
            const std::filesystem::path cmake_path = "CMakeLists.txt";
            
            std::cout << "Starting build process in " << build_mode << " mode...\n";
            
            try {
                // 1. Optionally update CMakeLists.txt from .libname
                if (update_cmake) {
                    if (!std::filesystem::exists(libname_path)) {
                        std::cerr << "Error: .libname file not found. Run 'ctc init' first.\n";
                        return 1;
                    }
                    std::cout << "Reading dependencies from .libname...\n";
                    std::vector<utils::DependencyEntry> dependencies = utils::read_libname(libname_path);
                    std::cout << "Updating CMakeLists.txt with project name '" << project_name << "' and dependencies...\n";
                    if (!utils::update_cmake_file(cmake_path, project_name, dependencies)) {
                        std::cerr << "Failed to update CMakeLists.txt\n";
                        return 1;
                    }
                    std::cout << "CMakeLists.txt updated successfully.\n";
                } else {
                    if (!std::filesystem::exists(cmake_path)) {
                        std::cerr << "Error: CMakeLists.txt not found. Run 'ctc apply' or use 'ctc run -U' first.\n";
                        return 1;
                    }
                }
                
                // 3. Create bin directory if it doesn't exist
                if (!utils::create_directory_if_not_exists(bin_dir)) {
                    std::cerr << "Failed to create bin directory\n";
                    return 1;
                }
                
                // 4. Create build directory
                std::error_code ec;
                if (std::filesystem::exists(build_dir, ec)) {
                    std::cout << "Removing existing build directory...\n";
                    std::filesystem::remove_all(build_dir, ec);
                    if (ec) {
                        std::cerr << "Warning: Failed to remove existing build directory: " << ec.message() << "\n";
                    }
                }
                
                if (!utils::create_directory_if_not_exists(build_dir)) {
                    std::cerr << "Failed to create build directory\n";
                    return 1;
                }
                std::cout << "Created build directory\n";
                
                // 5. Move to build directory
                std::filesystem::current_path(build_dir, ec);
                if (ec) {
                    std::cerr << "Failed to change to build directory: " << ec.message() << "\n";
                    return 1;
                }
                std::cout << "Changed to build directory\n";
                
                // 6. Execute cmake with build mode (CMAKE_TOOLCHAIN_FILE is embedded in CMakeLists if specified)
                std::cout << "Running cmake with build mode " << build_mode << "...\n";
                std::string cmake_command = "cmake -DCMAKE_BUILD_TYPE=" + build_mode + " ..";
                int cmake_result = std::system(cmake_command.c_str());
                if (cmake_result != 0) {
                    std::cerr << "CMake configuration failed\n";
                    std::filesystem::current_path(current_dir);
                    return 1;
                }
                
                // 7. Execute make (or cmake --build for cross-platform compatibility)
                std::cout << "Building project in " << build_mode << " mode...\n";
                std::string build_command = "cmake --build . --config " + build_mode;
                int build_result = std::system(build_command.c_str());
                if (build_result != 0) {
                    std::cerr << "Build failed\n";
                    std::filesystem::current_path(current_dir);
                    return 1;
                }
                
                // 8. Move back to original directory
                std::filesystem::current_path(current_dir, ec);
                if (ec) {
                    std::cerr << "Warning: Failed to return to original directory: " << ec.message() << "\n";
                }
                
                // 9. Copy executable to bin directory if it's not already there
                // The CMakeLists.txt template should handle this, but let's check
                if (std::filesystem::exists("build/bin")) {
                    std::cout << "Copying executables to bin directory...\n";
                    for (const auto& entry : std::filesystem::directory_iterator("build/bin")) {
                        if (entry.is_regular_file()) {
                            std::filesystem::path dest = bin_dir / entry.path().filename();
                            std::filesystem::copy_file(entry.path(), dest, 
                                std::filesystem::copy_options::overwrite_existing, ec);
                            if (!ec) {
                                std::cout << "Copied " << entry.path().filename() << " to bin/\n";
                            }
                        }
                    }
                } else {
                    // Look for executables in the build directory
                    for (const auto& entry : std::filesystem::directory_iterator("build")) {
                        if (entry.is_regular_file()) {
                            auto path = entry.path();
                            // Check if it's an executable (no extension on Unix, .exe on Windows)
                            if (!path.has_extension() || path.extension() == ".exe") {
                                std::filesystem::path dest = bin_dir / path.filename();
                                std::filesystem::copy_file(path, dest, 
                                    std::filesystem::copy_options::overwrite_existing, ec);
                                if (!ec) {
                                    std::cout << "Copied " << path.filename() << " to bin/\n";
                                }
                            }
                        }
                    }
                }
                
                // 10. Optionally delete build directory
                if (!keep_build_directory) {
                    std::cout << "Cleaning up build directory...\n";
                    std::filesystem::remove_all(build_dir, ec);
                    if (ec) {
                        std::cerr << "Warning: Failed to remove build directory: " << ec.message() << "\n";
                    } else {
                        std::cout << "Build directory cleaned up\n";
                    }
                } else {
                    std::cout << "Keeping build directory as requested (-k/--keep-build).\n";
                }
                
                std::cout << "Build completed successfully!\n";
                std::cout << "Executable(s) are now available in the bin/ directory.\n";
                
                return 0;
                
            } catch (const std::exception& e) {
                // Ensure we return to original directory in case of exception
                std::error_code ec;
                std::filesystem::current_path(current_dir, ec);
                
                std::cerr << "Error during build: " << e.what() << "\n";
                return 1;
            }
        }
    }
}
