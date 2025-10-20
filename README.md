# CTC - CMake Template Creator

CTC (CMake Template Creator) is a command-line tool that helps you quickly set up and manage C++ projects with a standardized directory structure and CMake build system.

## Features

- **Quick Project Setup**: Create a complete C++ project structure with a single command
- **Package Management**: Simple dependency tracking via `.libname` file
- **Automated Building**: One-command build process using CMake
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **C++17 Support**: Built with modern C++ standards

## Installation

### Build from Source

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd cmake-template-creater
   ```

2. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   cd ..
   ```

3. The `ctc` executable will be created in the `build/bin/` directory.

4. (Optional) Add the binary to your PATH for system-wide access.

## Usage

### Initialize a New Project

Create a new C++ project structure:

```bash
ctc init
```

This creates:
- `app/` - Main application source files
- `lib/` - Library source files  
- `include/` - Header files
- `bin/` - Built executables (created after build)
- `CMakeLists.txt` - CMake configuration
- `.gitignore` - Git ignore file for C++
- `.libname` - Package dependency tracking

To also create a README.md file for your project:
```bash
ctc init -r
```

### Manage Dependencies

CTC supports multiple types of dependencies similar to GCC/G++ flags, plus package components and custom link targets:

**Add dependencies:**
```bash
ctc install <package-name>          # Add CMake package (uses find_package)
ctc install <pkg>:<component>       # Add a specific package component (links as pkg::component)
ctc install <pkg> -c <comp> [-c <comp> ...]  # Add multiple components for a package
ctc install -L /path/to/libraries   # Add library search path
ctc install -l pthread              # Add library to link
ctc install -I /usr/local/include   # Add include directory
ctc install -T vcpkg/scripts/buildsystems/vcpkg.cmake  # Set CMAKE_TOOLCHAIN_FILE in CMakeLists.txt
ctc install -A <key>=<target>       # Add link override mapping:
                                    #   key = <pkg> or <pkg>:<component>
                                    #   target = custom target to use in target_link_libraries
```

**Remove dependencies:**
```bash
ctc uninstall <package-name>        # Remove CMake package
ctc uninstall <pkg>:<component>     # Remove package component
ctc uninstall <pkg> -c <comp> [-c <comp> ...]  # Remove one or more components
ctc uninstall -L /path/to/libraries # Remove library search path
ctc uninstall -l pthread            # Remove library
ctc uninstall -I /usr/local/include # Remove include directory
ctc uninstall -T vcpkg/scripts/buildsystems/vcpkg.cmake  # Remove toolchain file setting
ctc uninstall -A <key>=<target>     # Remove link override mapping
```

Dependencies are tracked in the `.libname` file with a structured format. Use `ctc apply` or `ctc run -U` to generate/update `CMakeLists.txt` content based on these dependencies. By default, `ctc run` builds using the existing `CMakeLists.txt`.

### List Dependencies

View all installed dependencies:
```bash
ctc list
```

This shows a nicely formatted overview of all packages, library paths, libraries, and include paths currently tracked in your `.libname` file, grouped by type for easy reading.

### Apply Dependencies

Update your CMakeLists.txt with current dependencies without building:
```bash
ctc apply
```

To specify a custom project name:
```bash
ctc apply -n my_project_name
```

This command:
- Reads all dependencies from `.libname` file
- Generates/updates `CMakeLists.txt` with proper CMake configuration
- Shows a summary of applied dependencies
- **Does NOT build** - just updates the configuration

This is useful when you want to update your CMakeLists.txt after adding/removing dependencies but don't want to build immediately.

### Build Your Project

Build your project with a single command:
```bash
ctc run                              # Build in Release mode (default), uses existing CMakeLists.txt
```

To specify a custom project name and build mode:
```bash
ctc run -n my_awesome_project        # Custom name, Release mode (uses existing CMakeLists.txt)
ctc run -m Debug                     # Default name, Debug mode
ctc run -n my_project -m Debug       # Custom name, Debug mode
ctc run -U -m Debug                  # Update CMakeLists.txt from .libname, then build in Debug
```

**Available build modes:**
- **Release** (default) - Optimized for performance
- **Debug** - Includes debugging information
- **MinSizeRel** - Optimized for minimum size
- **RelWithDebInfo** - Optimized with debugging information

This process:
1. Optionally updates `CMakeLists.txt` from `.libname` if `-U/--update-cmake` is used
2. Creates a `build` directory
3. Runs `cmake -DCMAKE_BUILD_TYPE=<mode> ..` to configure the build
4. Runs `cmake --build . --config <mode>` to compile
5. Copies the executable to the `bin/` directory
6. Removes the `build` directory (use `-k/--keep-build` to keep it)

### Get Help

```bash
ctc help
```

## Project Structure

After running `ctc init`, your project will have this structure:

```
your-project/
├── app/              # Main application source files (.cpp)
├── lib/              # Library source files (.cpp)  
├── include/          # Header files (.h, .hpp)
├── bin/              # Built executables (created after build)
├── CMakeLists.txt    # CMake configuration
├── .gitignore        # Git ignore patterns for C++
├── .libname          # Dependency tracking
└── README.md         # Project documentation (if -r flag used)
```

## Example Workflow

1. Create a new project:
   ```bash
   mkdir my-cpp-project
   cd my-cpp-project
   ctc init -r
   ```

2. Add your main source file:
   ```bash
   # Create app/main.cpp with your main() function
   echo '#include <iostream>\nint main() { std::cout << "Hello World!"; return 0; }' > app/main.cpp
   ```

3. Add dependencies as needed:
   ```bash
   ctc install boost              # Add Boost package
   ctc install -l pthread         # Link pthread library
   ctc install -I /usr/local/include  # Add custom include path
   ```

4. Check your dependencies:
   ```bash
   ctc list                       # View all installed dependencies
   ```

5. Apply dependencies to CMakeLists.txt:
   ```bash
   ctc apply -n my_awesome_project  # Update CMakeLists.txt without building
   ```

6. Build and run:
   ```bash
   ctc run                        # Build in Release mode (CMakeLists.txt already updated)
   ./bin/my_awesome_project
   ```

**Alternative (one-step build with options):**
   ```bash
   ctc run -U -n my_awesome_project -m Debug # Update CMakeLists and build in Debug mode
   ctc run -U -n my_awesome_project          # Update CMakeLists and build in Release mode
   ```

The `CMakeLists.txt` file can be generated/updated to include all your dependencies using `ctc apply` or `ctc run -U`!

## Automatic CMakeLists.txt Generation

The automatically generated `CMakeLists.txt` includes:
- C++17 standard requirement
- Project name (from `-n` flag or default)
- Toolchain file setting if installed via `ctc install -T <file>` (set before `project()`)
- `find_package()` calls for packages, trying `CONFIG` first and falling back to `MODULE`
  - If components were installed (e.g., `ctc install Boost -c program_options -c exception`), `COMPONENTS` are passed to `find_package`
- `link_directories()` for library paths added with `ctc install -L <path>`
- `include_directories()` for include paths added with `ctc install -I <path>`
- Automatic source file collection from `app/` and `lib/` (supports `.cpp`, `.cc`, `.c`)
- `target_link_libraries()` entries for:
  - packages as `pkg::pkg` when no components are specified
  - package components as `pkg::component`
  - custom overrides from `ctc install -A <key>=<target>` (e.g., `-A glfw3=glfw`)
- Executable output to `bin/` directory

**No manual CMakeLists.txt editing required!** Just use:
- `ctc install` to add dependencies
- `ctc apply` to update CMakeLists.txt (no build)
- `ctc run -U` to update CMakeLists.txt and build (defaults to Release mode)
- `ctc run` to build using the existing CMakeLists.txt
- `ctc run -m Debug` to build in Debug mode
- `ctc run -k` to keep the build directory after building

## Requirements

- CMake 3.17 or higher
- C++17 compatible compiler (GCC, Clang, MSVC)
- Make or Ninja build system

## Building CTC Itself

CTC uses CMake and is configured to build in Release mode by default for optimal performance. The CMakeLists.txt automatically detects your compiler and applies appropriate optimization flags.

For MSVC: Uses `/O2 /DNDEBUG`  
For GCC/Clang: Uses `-O3 -DNDEBUG`