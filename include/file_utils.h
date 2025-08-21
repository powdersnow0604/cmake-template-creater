#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace ctc {
    namespace utils {
        // Enhanced dependency entry structure
        struct DependencyEntry {
            enum Type { PACKAGE, LIBRARY_PATH, LIBRARY_NAME, INCLUDE_PATH, TOOLCHAIN_FILE };
            Type type;
            std::string value;
            
            std::string to_string() const;
            static DependencyEntry from_string(const std::string& str);
        };
        
        // File and directory utilities
        bool create_directory_if_not_exists(const std::filesystem::path& path);
        bool write_file(const std::filesystem::path& path, const std::string& content);
        std::string read_file(const std::filesystem::path& path);
        std::vector<std::string> read_lines(const std::filesystem::path& path);
        bool write_lines(const std::filesystem::path& path, const std::vector<std::string>& lines);
        
        // Enhanced libname utilities
        std::vector<DependencyEntry> read_libname(const std::filesystem::path& path);
        bool write_libname(const std::filesystem::path& path, const std::vector<DependencyEntry>& entries);
        bool add_dependency(const std::filesystem::path& libname_path, const DependencyEntry& entry);
        bool remove_dependency(const std::filesystem::path& libname_path, const DependencyEntry& entry);
        
        // CMakeLists.txt generation and modification
        std::string generate_cmake_content(const std::string& project_name, const std::vector<DependencyEntry>& dependencies);
        bool update_cmake_file(const std::filesystem::path& cmake_path, const std::string& project_name, const std::vector<DependencyEntry>& dependencies);
        
        // Template content generators
        std::string get_cmake_template();
        std::string get_gitignore_template();
        std::string get_readme_template();
    }
}
