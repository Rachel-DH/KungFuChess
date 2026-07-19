#include "asset_path.h"

#include <stdexcept>

#include <Windows.h>

namespace asset_path {

std::filesystem::path executable_dir() {
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        throw std::runtime_error("Failed to determine the running executable's path");
    }
    return std::filesystem::path(buffer, buffer + length).parent_path();
}

std::filesystem::path resolve(const std::filesystem::path& base_dir,
                               const std::filesystem::path& relative_path) {
    return (base_dir / relative_path).lexically_normal();
}

}  // namespace asset_path
