#include <set>
#include <filesystem>
#include <iostream>

#include "include/driver.hh"

#include <fstream>

#include "lexer.hh"
#include "include/util_logger.hh"

namespace fs = std::filesystem;
using namespace driver;


OptionParser::OptionParser(const int argc, char *argv[]) : argc_(argc), argv_(argv) {
}

OptionParser::~OptionParser() = default;

auto OptionParser::parse(config::Config &config) const -> bool {
  if (argc_ < 2) {
    std::cerr << "Error: No input files provided. Use --help for usage information." << std::endl;
    return false;
  }
  for (int i = 1; i < argc_; ++i) {
    std::string arg = argv_[i];
    // Handle flags/options
    if (arg[0] == '-') {
      if (arg == "-o") {
        if (++i < argc_) {
          config.output_file = argv_[i];
          config.output_specified = true;
        } else {
          std::cerr << "Error: Output file name missing after -o" << std::endl;
          return false;
        }
      } else if (arg == "-c") {
        config.build_mode = config::Config::BuildMode::COMPILE_ONLY;
      } else if (arg == "-E") {
        config.build_mode = config::Config::BuildMode::PREPROCESS;
      } else if (arg == "-S") {
        config.build_mode = config::Config::BuildMode::ASSEMBLE_ONLY;
      } else if (arg == "-fsyntax-only") {
        config.build_mode = config::Config::BuildMode::CHECK_SYNTAX_ONLY;
      } else if (arg == "-v" || arg == "--verbose") {
        config.verbose = true;
      } else if (arg == "-O0") {
        config.optimisation_level = 0;
      } else if (arg == "-O1") {
        config.optimisation_level = 1;
      } else if (arg == "-O2") {
        config.optimisation_level = 2;
      } else if (arg == "-O3") {
        config.optimisation_level = 3;
      } else if (arg == "-g") {
        config.debug = true;
      } else if (arg == "-w") {
        config.warnings = false;
      } else if (arg == "-Werror") {
        config.warnings_as_errors = true;
      } else if (arg == "--help") {
        config.show_help = true;
        return true;
      } else if (arg == "--version") {
        config.show_version = true;
        return true;
      } else if (arg == "-static") {
        config.static_linking = true;
      } else if (arg == "--keep-temp") {
        config.keep_temp_files = true;
      } else if (arg.substr(0, 2) == "-I") {
        if (arg.length() > 2) {
          config.include_paths.push_back(arg.substr(2));
        } else if (++i < argc_) {
          config.include_paths.emplace_back(argv_[i]);
        } else {
          std::cerr << "Error: Include path missing after -I" << std::endl;
          return false;
        }
      } else if (arg.substr(0, 2) == "-D") {
        std::string definition;
        if (arg.length() > 2) {
          definition = arg.substr(2);
        } else if (++i < argc_) {
          definition = argv_[i];
        } else {
          std::cerr << "Error: Macro definition missing after -D" << std::endl;
          return false;
        }
        size_t equal_pos = definition.find('=');
        if (equal_pos != std::string::npos) {
          config.defined_macros[definition.substr(0, equal_pos)] = definition.substr(equal_pos + 1);
        } else {
          config.defined_macros[definition] = "1"; // Define with default value of "1"
        }
      } else if (arg.substr(0, 2) == "-L") {
        if (arg.length() > 2) {
          config.library_paths.push_back(arg.substr(2));
        } else if (++i < argc_) {
          config.library_paths.push_back(argv_[i]);
        } else {
          std::cerr << "Error: Library path missing after -L" << std::endl;
          return false;
        }
      } else if (arg.substr(0, 2) == "-l") {
        if (arg.length() > 2) {
          config.libraries.push_back(arg.substr(2));
        } else if (++i < argc_) {
          config.libraries.push_back(argv_[i]);
        } else {
          std::cerr << "Error: Library name missing after -l" << std::endl;
          return false;
        }
      } else if (arg.substr(0, 5) == "-std=") {
        config.standard_version = arg.substr(5);
      } else if (arg.substr(0, 7) == "--arch=") {
        config.target_architecture = arg.substr(7);
      } else {
        std::cerr << "Warning: Unrecognised option: " << arg << std::endl;
      }
    } else {
      config.input_files.emplace_back(arg);
    }
  }

  //Validate configuration
  if (config.input_files.empty() && !config.show_help && !config.show_version) {
    std::cerr << "Error: No input files provided" << std::endl;
    return false;
  }

  // Set default output file if not specified
  if (!config.output_specified) {
    switch (config.build_mode) {
      case config::Config::BuildMode::COMPILE_ONLY: {
        if (!config.input_files.empty()) {
          const fs::path input_path{config.input_files.front()};
          config.output_file = input_path.stem().string() + ".o";
        }
        break;
      }
      case config::Config::BuildMode::ASSEMBLE_ONLY: {
        if (!config.input_files.empty()) {
          const fs::path input_path{config.input_files.front()};
          config.output_file = input_path.stem().string() + ".asm";
        }
        break;
      }
      default:
        // Default is a.out (already set in the config struct)
        break;
    }
  }

  return true;
}


auto driver::display_help(const std::string &prog_name) -> void {
  std::cout << "Usage: " << prog_name << " [options] file..." << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -c                    Compile and assemble, but do not link" << std::endl;
  std::cout << "  -E                    Preprocess only; do not compile, assemble or link" << std::endl;
  std::cout << "  -S                    Compile only; do not assemble or link" << std::endl;
  std::cout << "  -fsyntax-only         Check syntax only; do not compile, assemble or link" << std::endl;
  std::cout << "  -o <file>             Place the output into <file>" << std::endl;
  std::cout << "  -O<level>             Set optimisation level (0-3)" << std::endl;
  std::cout << "  -g                    Generate debugging information" << std::endl;
  std::cout << "  -w                    Disable all warnings" << std::endl;
  std::cout << "  -Werror               Treat warnings as errors" << std::endl;
  //std::cout << "  -std=<standard>       Specify language standard (e.g. 1)" << std::endl;
  std::cout << "  -I<dir>               Add directory to include search path" << std::endl;
  //std::cout << "  -D<macro>[=<val>]     Define a preprocessor macro" << std::endl;
  std::cout << "  -L<dir>               Add directory to library search path" << std::endl;
  std::cout << "  -l<lib>               Link with library <lib>" << std::endl;
  std::cout << "  -static               Link statically rather than dynamically" << std::endl;
  //std::cout << "  --arch=<arch>         Specify target architecture" << std::endl;
  std::cout << "  --keep-temp           Keep temporary files" << std::endl;
  std::cout << "  -v, --verbose         Enable verbose output" << std::endl;
  std::cout << "  --help                Display this information" << std::endl;
  std::cout << "  --version             Display compiler version information" << std::endl;
}

auto driver::display_version() -> void {
  std::cout << "Argon Compiler (argc) v0.0.1" << std::endl;
  std::cout << "Copyright (C) 2025 Yasu Fadhili" << std::endl;
  std::cout << "This is free software; see the source for copying conditions." << std::endl;
}

auto driver::validate_input_files(const config::Config &config) -> bool {
  std::set<std::string> valid_extensions{".ar", ".arg", ".argon"};
  for (const auto &file: config.input_files) {
    if (!fs::exists(file)) {
      LOG_ERROR("Error: Input file '" + file + "' does not exist");
      return false;
    }
    std::string extension{fs::path(file).extension().string()};
    if (valid_extensions.find(extension) == valid_extensions.end()) {
      LOG_ERROR("Warning: File '" + file + "' has unrecognised extension '" + extension + "'");
    }
  }
  return true;
}

auto driver::cleanup_temp_files(const config::Config &config) -> void {
  if (!config.keep_temp_files) {
    for (const auto &file: config.temp_files) {
      //std::remove(file.c_str());
      if (fs::exists(file)) {
        try {
          fs::remove(file);
            LOG_INFO_IF(config.verbose, "Removed temporary file: " + file);
        } catch (const std::exception &e) {
          LOG_WARNING("Failed to remove temporary file '" + file);
        }
      }
    }
  }
}
