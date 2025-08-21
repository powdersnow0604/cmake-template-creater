#include <iostream>
#include <vector>
#include <string>
#include "commands.h"

void print_help() {
    std::cout << "CTC - CMake Template Creator\n\n";
    std::cout << "Usage: ctc <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  init [-r]                    Create new project structure (use -r to include README.md)\n\n";
    std::cout << "  install <package>            Add package to .libname file\n";
    std::cout << "  install -L <library-path>    Add library search path\n";
    std::cout << "  install -l <library-name>    Add library to link\n";
    std::cout << "  install -I <include-path>    Add include directory\n";
    std::cout << "  install -T <toolchain-file>  Set CMAKE_TOOLCHAIN_FILE path\n\n";
    std::cout << "  uninstall <package>          Remove package from .libname file\n";
    std::cout << "  uninstall -L <library-path>  Remove library search path\n";
    std::cout << "  uninstall -l <library-name>  Remove library to link\n";
    std::cout << "  uninstall -I <include-path>  Remove include directory\n";
    std::cout << "  uninstall -T <toolchain-file> Remove CMAKE_TOOLCHAIN_FILE path\n\n";
    std::cout << "  apply [-n <name>]            Update CMakeLists.txt with dependencies (no build)\n";
    std::cout << "                               Use -n to specify project name (default: my_project)\n\n";
    std::cout << "  run [-n <name>] [-m <mode>] [-k|--keep-build]  Build project using CMake (auto-updates CMakeLists.txt)\n";
    std::cout << "                               Use -n to specify project name (default: my_project)\n";
    std::cout << "                               Use -m to specify build mode (default: Release)\n";
    std::cout << "                               Use -k/--keep-build to keep the build directory after build\n";
    std::cout << "                               Valid modes: Debug, Release, MinSizeRel, RelWithDebInfo\n\n";
    std::cout << "  list                         Show all dependencies in .libname file\n\n";
    std::cout << "  help                         Show this help message\n\n";
    std::cout << "Note: Use 'ctc apply' to update CMakeLists.txt, or 'ctc run' to update and build.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;
    
    // Collect remaining arguments
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    try {
        if (command == "init") {
            return ctc::commands::init_command(args);
        } else if (command == "install") {
            return ctc::commands::install_command(args);
        } else if (command == "uninstall") {
            return ctc::commands::uninstall_command(args);
        } else if (command == "apply") {
            return ctc::commands::apply_command(args);
        } else if (command == "run") {
            return ctc::commands::run_command(args);
        } else if (command == "list") {
            return ctc::commands::list_command(args);
        } else if (command == "help" || command == "--help" || command == "-h") {
            print_help();
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << "\n";
            print_help();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
