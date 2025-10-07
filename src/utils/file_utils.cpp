#include "file_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <set>

namespace ctc {
    namespace utils {
        
        // DependencyEntry implementation
        std::string DependencyEntry::to_string() const {
            switch (type) {
                case PACKAGE: return "PKG:" + value;
                case LIBRARY_PATH: return "LIBPATH:" + value;
                case LIBRARY_NAME: return "LIB:" + value;
                case INCLUDE_PATH: return "INCPATH:" + value;
                case TOOLCHAIN_FILE: return "TOOLCHAIN:" + value;
                case LINK_OVERRIDE: return "LINKOVR:" + value; // format: pkg[:component]=customTarget
                case PACKAGE_COMPONENT: return "PKGCOMP:" + value; // format: pkg:component
                default: return value;
            }
        }
        
        DependencyEntry DependencyEntry::from_string(const std::string& str) {
            DependencyEntry entry;
            if (str.substr(0, 4) == "PKG:") {
                entry.type = PACKAGE;
                entry.value = str.substr(4);
            } else if (str.substr(0, 8) == "LIBPATH:") {
                entry.type = LIBRARY_PATH;
                entry.value = str.substr(8);
            } else if (str.substr(0, 4) == "LIB:") {
                entry.type = LIBRARY_NAME;
                entry.value = str.substr(4);
            } else if (str.substr(0, 8) == "INCPATH:") {
                entry.type = INCLUDE_PATH;
                entry.value = str.substr(8);
            } else if (str.substr(0, 10) == "TOOLCHAIN:") {
                entry.type = TOOLCHAIN_FILE;
                entry.value = str.substr(10);
            } else if (str.substr(0, 8) == "LINKOVR:") {
                entry.type = LINK_OVERRIDE;
                entry.value = str.substr(8); // expected: pkg[:component]=customTarget
            } else if (str.substr(0, 8) == "PKGCOMP:") {
                entry.type = PACKAGE_COMPONENT;
                entry.value = str.substr(8); // expected: pkg:component
            } else {
                // Legacy format - assume it's a package
                entry.type = PACKAGE;
                entry.value = str;
            }
            return entry;
        }

        bool create_directory_if_not_exists(const std::filesystem::path& path) {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec)) {
                return std::filesystem::create_directories(path, ec);
            }
            return true;
        }

        bool write_file(const std::filesystem::path& path, const std::string& content) {
            std::ofstream file(path);
            if (!file.is_open()) {
                return false;
            }
            file << content;
            return file.good();
        }

        std::string read_file(const std::filesystem::path& path) {
            std::ifstream file(path);
            if (!file.is_open()) {
                return "";
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        std::vector<std::string> read_lines(const std::filesystem::path& path) {
            std::vector<std::string> lines;
            std::ifstream file(path);
            if (!file.is_open()) {
                return lines;
            }
            
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            return lines;
        }

        bool write_lines(const std::filesystem::path& path, const std::vector<std::string>& lines) {
            std::ofstream file(path);
            if (!file.is_open()) {
                return false;
            }
            
            for (const auto& line : lines) {
                file << line << "\n";
            }
            return file.good();
        }
        
        // Enhanced libname utilities
        std::vector<DependencyEntry> read_libname(const std::filesystem::path& path) {
            std::vector<DependencyEntry> entries;
            std::vector<std::string> lines = read_lines(path);
            
            for (const auto& line : lines) {
                if (!line.empty()) {
                    entries.push_back(DependencyEntry::from_string(line));
                }
            }
            return entries;
        }
        
        bool write_libname(const std::filesystem::path& path, const std::vector<DependencyEntry>& entries) {
            std::vector<std::string> lines;
            for (const auto& entry : entries) {
                lines.push_back(entry.to_string());
            }
            return write_lines(path, lines);
        }
        
        bool add_dependency(const std::filesystem::path& libname_path, const DependencyEntry& entry) {
            std::vector<DependencyEntry> entries = read_libname(libname_path);
            
            // Check if entry already exists
            for (const auto& existing : entries) {
                if (existing.type == entry.type && existing.value == entry.value) {
                    return true; // Already exists
                }
            }
            
            entries.push_back(entry);
            return write_libname(libname_path, entries);
        }
        
        bool remove_dependency(const std::filesystem::path& libname_path, const DependencyEntry& entry) {
            std::vector<DependencyEntry> entries = read_libname(libname_path);
            
            auto it = std::remove_if(entries.begin(), entries.end(),
                [&entry](const DependencyEntry& e) {
                    return e.type == entry.type && e.value == entry.value;
                });
            
            if (it != entries.end()) {
                entries.erase(it, entries.end());
                return write_libname(libname_path, entries);
            }
            
            return true; // Nothing to remove
        }
        
        // CMakeLists.txt generation and modification
        std::string generate_cmake_content(const std::string& project_name, const std::vector<DependencyEntry>& dependencies) {
            std::stringstream cmake_content;
            
            // Process dependencies
            std::vector<std::string> packages;
            std::vector<std::string> lib_paths;
            std::vector<std::string> lib_names;
            std::vector<std::string> inc_paths;
            // map of package -> list of components
            std::map<std::string, std::vector<std::string>> package_to_components;
            // map of pkg or pkg:component -> custom link name
            std::map<std::string, std::string> link_overrides;
            // toolchain file (if specified)
            std::string toolchain_file_path;
            
            for (const auto& dep : dependencies) {
                switch (dep.type) {
                    case DependencyEntry::PACKAGE:
                        packages.push_back(dep.value);
                        break;
                    case DependencyEntry::LIBRARY_PATH:
                        lib_paths.push_back(dep.value);
                        break;
                    case DependencyEntry::LIBRARY_NAME:
                        lib_names.push_back(dep.value);
                        break;
                    case DependencyEntry::INCLUDE_PATH:
                        inc_paths.push_back(dep.value);
                        break;
                    case DependencyEntry::TOOLCHAIN_FILE:
                        if (toolchain_file_path.empty()) {
                            toolchain_file_path = dep.value;
                        }
                        break;
                    case DependencyEntry::LINK_OVERRIDE: {
                        auto eq = dep.value.find('=');
                        if (eq != std::string::npos) {
                            std::string key = dep.value.substr(0, eq);
                            std::string target = dep.value.substr(eq + 1);
                            link_overrides[key] = target;
                        }
                        break;
                    }
                    case DependencyEntry::PACKAGE_COMPONENT: {
                        auto sep = dep.value.find(':');
                        if (sep != std::string::npos) {
                            std::string pkg = dep.value.substr(0, sep);
                            std::string comp = dep.value.substr(sep + 1);
                            package_to_components[pkg].push_back(comp);
                        }
                        break;
                    }
                }
            }

            // Managed header
            cmake_content << "# === CTC MANAGED SECTION (auto-generated) ===\n";
            cmake_content << "# Edits in this section may be overwritten by 'ctc apply' or 'ctc run'.\n\n";

            // Basic project setup
            cmake_content << "cmake_minimum_required(VERSION 3.17)\n";
            
            // If toolchain specified, set it before project()
            if (!toolchain_file_path.empty()) {
                cmake_content << "\n# Toolchain\n";
                cmake_content << "set(CMAKE_TOOLCHAIN_FILE \"" << toolchain_file_path << "\" CACHE FILEPATH \"Toolchain file\")\n\n";
            }

            cmake_content << "project(" << project_name << " VERSION 1.0.0)\n\n";
            
            // Set C++17 standard
            cmake_content << "# Set C++17 standard\n";
            cmake_content << "set(CMAKE_CXX_STANDARD 17)\n";
            cmake_content << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

            // Add find_package calls: try CONFIG first (with COMPONENTS if present), fallback to MODULE (with COMPONENTS if present)
            if (!packages.empty() || !package_to_components.empty()) {
                cmake_content << "# Find packages (try CONFIG first, fallback to MODULE)\n";
                // unique set of all packages involved (plain packages + packages with components)
                std::set<std::string> all_pkgs(packages.begin(), packages.end());
                for (const auto& kv : package_to_components) { all_pkgs.insert(kv.first); }
                for (const auto& pkg : all_pkgs) {
                    const auto comps_it = package_to_components.find(pkg);
                    const bool has_components = comps_it != package_to_components.end() && !comps_it->second.empty();
                    // CONFIG attempt
                    cmake_content << "find_package(" << pkg << " QUIET CONFIG";
                    if (has_components) {
                        cmake_content << " COMPONENTS";
                        for (const auto& comp : comps_it->second) {
                            cmake_content << " " << comp;
                        }
                    }
                    cmake_content << ")\n";
                    // Fallback to MODULE
                    cmake_content << "if(NOT " << pkg << "_FOUND)\n";
                    cmake_content << "    find_package(" << pkg << " REQUIRED MODULE";
                    if (has_components) {
                        cmake_content << " COMPONENTS";
                        for (const auto& comp : comps_it->second) {
                            cmake_content << " " << comp;
                        }
                    }
                    cmake_content << ")\n";
                    cmake_content << "endif()\n";
                }
                cmake_content << "\n";
            }
            
            // Add library search paths
            if (!lib_paths.empty()) {
                cmake_content << "# Library search paths\n";
                for (const auto& path : lib_paths) {
                    cmake_content << "link_directories(" << path << ")\n";
                }
                cmake_content << "\n";
            }
            
            // Add include directories
            cmake_content << "# Include directories\n";
            cmake_content << "include_directories(include)\n";
            if (!inc_paths.empty()) {
                for (const auto& path : inc_paths) {
                    cmake_content << "include_directories(" << path << ")\n";
                }
            }
            cmake_content << "\n";
            
            // Collect source files
            cmake_content << "# Collect source files from lib directory\n";
            cmake_content << "file(GLOB_RECURSE LIB_SOURCES \"lib/*.cpp\" \"lib/*.cc\" \"lib/*.c\")\n\n";
            cmake_content << "# Collect source files from app directory\n";
            cmake_content << "file(GLOB_RECURSE APP_SOURCES \"app/*.cpp\" \"app/*.cc\" \"app/*.c\")\n\n";
            
            // Create executable
            cmake_content << "# Create executable\n";
            cmake_content << "add_executable(${PROJECT_NAME} ${APP_SOURCES} ${LIB_SOURCES})\n\n";
            
            // Set output directory
            cmake_content << "# Set output directory\n";
            cmake_content << "set_target_properties(${PROJECT_NAME} PROPERTIES\n";
            cmake_content << "    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin\n";
            cmake_content << ")\n\n";
            
            // Link libraries
            if (!lib_names.empty() || !packages.empty() || !package_to_components.empty()) {
                cmake_content << "# Link libraries\n";
                cmake_content << "target_link_libraries(${PROJECT_NAME}";
                
                // Add package libraries honoring link overrides: either pkg::component (if components specified) or pkg::pkg
                {
                    std::set<std::string> printed_pkg_targets;
                    // packages with components
                    for (const auto& kv : package_to_components) {
                        const auto& pkg = kv.first;
                        for (const auto& comp : kv.second) {
                            std::string key = pkg + ":" + comp;
                            auto it = link_overrides.find(key);
                            if (it != link_overrides.end()) {
                                cmake_content << " " << it->second;
                            } else {
                                cmake_content << " " << pkg << "::" << comp;
                            }
                        }
                        printed_pkg_targets.insert(pkg);
                    }
                    // plain packages
                    for (const auto& pkg : packages) {
                        if (printed_pkg_targets.find(pkg) == printed_pkg_targets.end()) {
                            auto it = link_overrides.find(pkg);
                            if (it != link_overrides.end()) {
                                cmake_content << " " << it->second;
                            } else {
                                cmake_content << " " << pkg << "::" << pkg;
                            }
                        }
                    }
                }
                
                // Add library names
                for (const auto& lib : lib_names) {
                    cmake_content << " " << lib;
                }
                
                cmake_content << ")\n";
            }
            
            return cmake_content.str();
        }
        
        bool update_cmake_file(const std::filesystem::path& cmake_path, const std::string& project_name, const std::vector<DependencyEntry>& dependencies) {
            const std::string user_marker = "# === CTC USER SECTION (not modified by ctc) ===";
            const std::string user_placeholder = R"(# === CTC USER SECTION (not modified by ctc) ===
# Add any custom CMake logic below. This section is preserved by ctc.
)";

            std::string managed_content = generate_cmake_content(project_name, dependencies);

            // Preserve existing user section if present
            std::string existing = read_file(cmake_path);
            std::string user_section;
            if (!existing.empty()) {
                size_t pos = existing.find(user_marker);
                if (pos != std::string::npos) {
                    user_section = existing.substr(pos);
                }
            }
            if (user_section.empty()) {
                user_section = user_placeholder;
            }

            std::string final_content = managed_content + "\n" + user_section;
            return write_file(cmake_path, final_content);
        }

        std::string get_cmake_template() {
            return R"(cmake_minimum_required(VERSION 3.17)
project(my_project VERSION 1.0.0)

# Set C++17 standard  
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages (add your packages here)
# Example: find_package(PkgConfig REQUIRED)

# Include directories
include_directories(include)

# Collect source files from lib directory
file(GLOB_RECURSE LIB_SOURCES "lib/*.cpp" "lib/*.cc" "lib/*.c")

# Collect source files from app directory  
file(GLOB_RECURSE APP_SOURCES "app/*.cpp" "app/*.cc" "app/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${LIB_SOURCES})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

# Link libraries (add your libraries here)
# Example: target_link_libraries(${PROJECT_NAME} your_library)
)";
        }

        std::string get_gitignore_template() {
            return R"(# Prerequisites
*.d

# Compiled Object files
*.slo
*.lo
*.o
*.obj

# Precompiled Headers
*.gch
*.pch

# Compiled Dynamic libraries
*.so
*.dylib
*.dll

# Fortran module files
*.mod
*.smod

# Compiled Static libraries
*.lai
*.la
*.a
*.lib

# Executables
*.exe
*.out
*.app

# Build directories
build/
Build/
BUILD/
debug/
Debug/
release/
Release/

# CMake
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
Makefile
*.cmake

# IDE files
.vscode/
.vs/
*.vcxproj*
*.sln
*.suo
*.user
*.ncb
*.aps
*.plg
*.opt
*.clw
*.tmp
*.log

# OS generated files
.DS_Store
.DS_Store?
._*
.Spotlight-V100
.Trashes
ehthumbs.db
Thumbs.db
)";
        }

        std::string get_readme_template() {
            return R"(# My Project

## Description
A C++ project created with CTC (CMake Template Creator).

## Build Instructions

1. Use CTC to build the project:
   ```bash
   ctc run
   ```

2. Or build manually:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   cd ..
   ```

The executable will be placed in the `bin` directory.

## Project Structure

- `app/` - Main application source files
- `lib/` - Library source files  
- `include/` - Header files
- `bin/` - Built executables (created after build)

## Dependencies

Dependencies are managed in the `.libname` file. Use:
- `ctc install <package>` to add dependencies
- `ctc uninstall <package>` to remove dependencies
)";
        }
    }
}
