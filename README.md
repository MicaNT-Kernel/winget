# Sovereign Windows Package Manager (`winget`)

> **"A clean-room, zero-telemetry, freestanding C++23 package management engine."**  
> An open-source, ultra-lightweight implementation of the Windows Package Manager interface, manifest parser, dependency DAG solver, and command-line execution broker.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Clean Room: Certified](https://img.shields.io/badge/Clean%20Room-Certified-success.svg)](docs/CLEAN_ROOM_PROVENANCE.md)
[![Standard: C++23](https://img.shields.io/badge/Language-C%2B%2B23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Build: CMake](https://img.shields.io/badge/Build-CMake%203.25%2B-green.svg)]()
[![Schema: v1.6.0](https://img.shields.io/badge/Schema-v1.6.0%20Compliant-purple.svg)](https://github.com/microsoft/winget-pkgs)
[![Telemetry: 0%](https://img.shields.io/badge/Telemetry-0%25%20(None)-brightgreen.svg)]()

---

## 1. Why Sovereign `winget`?

The official `winget-cli` is weighed down by extensive corporate dependencies:
- **C++/WinRT** and heavy Windows Runtime projected headers
- **WIL** (Windows Implementation Libraries)
- Monolithic external libraries: `SQLite3`, `yaml-cpp`, `jsoncpp`, `libcurl`
- Pervasive telemetry probes (`TraceLoggingWrite`, Azure App Center)
- Hundreds of megabytes of compile-time and link-time overhead

**Sovereign winget** strips away the entire web of dependencies. It is authored in modern **ISO C++23** as a freestanding, single-header library and a lightning-fast CLI that compiles in seconds with **zero external dependencies** and **zero telemetry**.

| Feature | Upstream `winget-cli` | Sovereign `winget` |
| :--- | :--- | :--- |
| **Dependencies** | C++/WinRT, WIL, SQLite3, yaml-cpp, curl | **0** (Standard ISO C++23 only) |
| **Crypto Provider** | Windows BCrypt / CNG | **Freestanding NIST FIPS 180-4 SHA-256** |
| **Build Time** | Several minutes (heavy vcpkg/MSBuild) | **~3 seconds** (Ninja / MSVC / Clang) |
| **Telemetry & Tracking**| Built-in (TraceLogging, Azure AppCenter) | **0% (Disabled by design)** |
| **Binary Footprint** | ~15 MB – 30 MB across DLLs | **< 150 KB** standalone executable |
| **Manifest Compatibility**| Schema v1.6.0 YAML/JSON | **Schema v1.6.0 YAML/JSON compliant** |
| **Dependency Resolution** | Internal COM graph resolver | **Topological DAG with cycle detection** |
| **Platforms** | Windows 10/11 only | **Windows, Linux, macOS** |

---

## 2. Key Architecture

### 1. Freestanding NIST FIPS 180-4 Cryptographic Engine
Integrated clean-room implementation of the SHA-256 secure hash algorithm (`winget::Sha256`). Computes package checksums on the fly and verifies downloaded binaries against manifest digests with single-bit tamper detection.

### 2. Package Manifest Engine (Schema v1.6.0)
Parser and schema validator compliant with the official community catalog (`microsoft/winget-pkgs`):
- `PackageIdentifier`, `PackageVersion`, `PackageName`, `Publisher`, `License`, `Moniker`
- Multi-installer support: `msix`, `appx`, `exe`, `msi`, `zip`, `portable`, `inno`, `nullsoft`
- Architecture filtering: `x64`, `arm64`, `x86`, `neutral`
- Execution scopes: `User`, `Machine`

### 3. Topological Dependency Resolution (DAG)
Resolves multi-level application dependencies automatically. If package `A` depends on package `B`, the topological sorter stages `B` prior to `A`. Cyclic dependencies are immediately rejected (`WINGET_INST_E_DEPENDENCY_CYCLE`).

### 4. Clean-Room Win32 C ABI Parity
Exposes standard Win32 C ABI functions for drop-in integration:
- `WinGetCreatePackageManager`
- `WinGetFindPackages`
- `WinGetInstallPackage`
- `WinGetUninstallPackage`
- `WinGetGetPackageManifest`
- `WinGetVerifyPackageHash`
- `WinGetRegisterSource`
- `WinGetUnregisterSource`
- `WinGetGetInstalledCount`

---

## 3. Quickstart

### Include as a Header-Only Library

```cpp
#include <winget.hpp>
#include <iostream>

using namespace winget;

int main() {
    auto& mgr = WinGetManager::Instance();

    // 1. Search for a package
    auto results = mgr.search("terminal");
    for (const auto* pkg : results) {
        std::cout << "Found: " << pkg->packageName << " (" << pkg->packageVersion << ")\n";
    }

    // 2. Install package with automatic dependency resolution
    std::vector<std::string> log;
    if (mgr.install("Microsoft.WindowsTerminal", log) == WINGET_S_OK) {
        std::cout << "Installation succeeded!\n";
    }

    return 0;
}
```

---

## 4. Command-Line Interface (`winget`)

```text
usage: winget [<command>] [<options>]

The following commands are available:
  status      Show current repository posture and zero-telemetry integrity
  search      Find and show basic info of packages
  show        Show details about a package
  install     Install the specified package
  uninstall   Uninstall the specified package
  list        Display installed packages
  upgrade     Display available upgrades or upgrade a package
  pin         Manage package version pins
  source      Manage sources of packages
  hash        Helper to hash installer files using NIST SHA-256
  validate    Validate a manifest file against schema v1.6.0
```

### Examples

```bash
# Check posture
winget status

# Search available packages
winget search git

# Show manifest details and dependencies
winget show Microsoft.PowerToys

# Install package (auto-stages VCRedist dependency)
winget install Microsoft.PowerToys

# Pin a package version to prevent upgrades
winget pin add Microsoft.PowerToys

# Compute SHA-256 hash
winget hash installer.exe

# Validate a manifest file
winget validate manifest.yaml
```

---

## 5. Building & Testing

Requires standard CMake (3.25+) and any C++23 compiler (MSVC 19.38+, Clang 17+, or GCC 13+):

```bash
# Clone
git clone https://github.com/MicaNT-Kernel/winget.git
cd winget

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build CLI and tests
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

---

## 6. License & Legal Notice

Licensed under the [MIT License](LICENSE).

*Microsoft, Windows, and Windows Package Manager are trademarks of Microsoft Corporation. This project is an independent clean-room implementation authored from first principles and publicly published open specifications solely for interoperability (Google LLC v. Oracle America, Inc.).*
