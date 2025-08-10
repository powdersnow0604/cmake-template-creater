#pragma once

#include <string>
#include <vector>

namespace ctc {
    namespace commands {
        // Command functions
        int init_command(const std::vector<std::string>& args);
        int install_command(const std::vector<std::string>& args);
        int uninstall_command(const std::vector<std::string>& args);
        int run_command(const std::vector<std::string>& args);
        int list_command(const std::vector<std::string>& args);
        int apply_command(const std::vector<std::string>& args);
    }
}
