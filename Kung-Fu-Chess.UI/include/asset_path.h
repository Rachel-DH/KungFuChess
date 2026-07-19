#pragma once

#include <filesystem>

// Resolves asset paths relative to the running executable, so assets load
// correctly regardless of the process's current working directory.
namespace asset_path {

// Full path to the directory containing the running executable.
// Throws std::runtime_error if it can't be determined.
std::filesystem::path executable_dir();

// Joins base_dir and relative_path and normalizes away ".." segments.
std::filesystem::path resolve(const std::filesystem::path& base_dir,
                               const std::filesystem::path& relative_path);

}  // namespace asset_path
