// ============================================================================
// Sovereign Windows Package Manager Unit Test Suite
// (tests/test_winget.cpp)
// ============================================================================

#include "winget.hpp"
#include <iostream>
#include <cassert>

using namespace winget;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] ASSERTION FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            return 1; \
        } \
    } while(0)

int main() {
    std::cout << "========================================================================\n"
              << "   Sovereign Windows Package Manager (winget) Test Suite               \n"
              << "========================================================================\n";

    // 1. NIST SHA-256 Vector Verification
    std::cout << "[Stage 1] Verifying NIST FIPS 180-4 SHA-256 Test Vectors...\n";
    {
        std::string emptyHash = Sha256::hashString("");
        TEST_ASSERT(emptyHash == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "Empty string hash must match NIST vector");

        std::string abcHash = Sha256::hashString("abc");
        TEST_ASSERT(abcHash == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "String 'abc' hash must match NIST vector");
        std::cout << "  [PASS] Cryptographic SHA-256 engine matches standard test vectors.\n";
    }

    // 2. Tamper Detection
    std::cout << "[Stage 2] Verifying Cryptographic Tamper Detection...\n";
    {
        const char* fakePayload = "Legitimate Binary Content";
        std::string validHash = Sha256::hashString(fakePayload);
        TEST_ASSERT(validHash != "0000000000000000000000000000000000000000000000000000000000000000", "Valid hash calculated");

        const char* tamperedPayload = "Legitimate Binary ContenT";
        std::string tamperedHash = Sha256::hashString(tamperedPayload);
        TEST_ASSERT(validHash != tamperedHash, "Tampered payload must produce distinct hash digest");
        std::cout << "  [PASS] Single-bit modification detected by SHA-256 avalanche effect.\n";
    }

    // 3. Manifest Parser (Schema v1.6.0)
    std::cout << "[Stage 3] Parsing YAML Schema v1.6.0 Package Manifest...\n";
    {
        std::string yamlManifest =
            "PackageIdentifier: TestOrg.TestPackage\n"
            "PackageVersion: 2.1.0\n"
            "PackageName: Test Package Tool\n"
            "Publisher: Test Organization\n"
            "License: MIT\n"
            "Moniker: testpkg\n"
            "Installers:\n"
            "  - Architecture: x64\n"
            "    InstallerType: msix\n"
            "    InstallerUrl: https://example.com/test.msix\n"
            "    InstallerSha256: 499d45395a4c6fdf1904a43415c1e55ec818b2ab4e83c213ad4ff299555cb5b8\n";

        PackageManifest manifest;
        TEST_ASSERT(ManifestParser::parse(yamlManifest, manifest), "Manifest parser must succeed");
        TEST_ASSERT(manifest.packageIdentifier == "TestOrg.TestPackage", "Identifier match");
        TEST_ASSERT(manifest.packageVersion == "2.1.0", "Version match");
        TEST_ASSERT(manifest.installers.size() == 1, "Installers parsed");
        TEST_ASSERT(manifest.installers[0].architecture == PackageArchitecture::X64, "Arch x64");
        TEST_ASSERT(manifest.installers[0].architectureType == InstallerType::Msix, "Type MSIX");
        std::cout << "  [PASS] YAML manifest parsed and validated.\n";
    }

    // 4. Dependency DAG Resolution
    std::cout << "[Stage 4] Testing Directed Acyclic Graph (DAG) Dependency Resolution...\n";
    {
        DependencyResolver::PackageMap catalog;
        
        PackageManifest pkgA;
        pkgA.packageIdentifier = "App.A";
        pkgA.packageVersion = "1.0.0";
        pkgA.packageName = "App A";
        pkgA.publisher = "Pub";
        DependencyInfo depB;
        depB.packageIdentifier = "Lib.B";
        pkgA.dependencies.push_back(depB);

        PackageManifest pkgB;
        pkgB.packageIdentifier = "Lib.B";
        pkgB.packageVersion = "1.0.0";
        pkgB.packageName = "Lib B";
        pkgB.publisher = "Pub";

        catalog[pkgA.packageIdentifier] = pkgA;
        catalog[pkgB.packageIdentifier] = pkgB;

        std::vector<std::string> order;
        std::string err;
        TEST_ASSERT(DependencyResolver::resolve("App.A", catalog, order, err), "DAG resolution must succeed");
        TEST_ASSERT(order.size() == 2, "2 packages in order");
        TEST_ASSERT(order[0] == "Lib.B", "Dependency Lib.B must be staged first");
        TEST_ASSERT(order[1] == "App.A", "Root package App.A must be staged second");
        std::cout << "  [PASS] Topological ordering verified (Lib.B -> App.A).\n";
    }

    // 5. Circular Dependency Detection
    std::cout << "[Stage 5] Detecting Circular Dependency Locks...\n";
    {
        DependencyResolver::PackageMap catalog;
        
        PackageManifest pkgA;
        pkgA.packageIdentifier = "Loop.A";
        pkgA.packageVersion = "1.0.0";
        pkgA.packageName = "Loop A";
        pkgA.publisher = "Pub";
        DependencyInfo depB;
        depB.packageIdentifier = "Loop.B";
        pkgA.dependencies.push_back(depB);

        PackageManifest pkgB;
        pkgB.packageIdentifier = "Loop.B";
        pkgB.packageVersion = "1.0.0";
        pkgB.packageName = "Loop B";
        pkgB.publisher = "Pub";
        DependencyInfo depA;
        depA.packageIdentifier = "Loop.A";
        pkgB.dependencies.push_back(depA);

        catalog[pkgA.packageIdentifier] = pkgA;
        catalog[pkgB.packageIdentifier] = pkgB;

        std::vector<std::string> order;
        std::string err;
        TEST_ASSERT(!DependencyResolver::resolve("Loop.A", catalog, order, err), "Circular dependency must be rejected");
        TEST_ASSERT(err.find("Circular dependency detected") != std::string::npos, "Error must describe cycle");
        std::cout << "  [PASS] Circular dependency successfully blocked.\n";
    }

    // 6. Installation Lifecycle
    std::cout << "[Stage 6] Executing Package Installation Lifecycle...\n";
    auto& mgr = WinGetManager::Instance();
    std::vector<std::string> log;
    TEST_ASSERT(mgr.install("Microsoft.WindowsTerminal", log) == WINGET_S_OK, "Install Windows Terminal");
    TEST_ASSERT(mgr.getInstalledCount() >= 1, "Package count >= 1");
    std::cout << "  [PASS] Microsoft.WindowsTerminal installed successfully.\n";

    // 7. Duplicate Install Prevention
    std::cout << "[Stage 7] Verifying Duplicate Installation Prevention...\n";
    log.clear();
    TEST_ASSERT(mgr.install("Microsoft.WindowsTerminal", log) == WINGET_INST_E_ALREADY_INSTALLED, "Duplicate install rejected");
    std::cout << "  [PASS] Re-installation of active package prevented.\n";

    // 8. Auto-Staging Dependencies
    std::cout << "[Stage 8] Auto-Staging Framework Dependencies (PowerToys -> VCRedist)...\n";
    log.clear();
    TEST_ASSERT(mgr.install("Microsoft.PowerToys", log) == WINGET_S_OK, "Install PowerToys");
    bool stagedDep = false;
    for (const auto& line : log) {
        if (line.find("Staged dependency: Microsoft.VCRedist.2015+.x64") != std::string::npos) {
            stagedDep = true;
            break;
        }
    }
    TEST_ASSERT(stagedDep, "Dependency Microsoft.VCRedist.2015+.x64 must be auto-staged");
    std::cout << "  [PASS] Dependency DAG successfully auto-resolved in lifecycle broker.\n";

    // 9. Version Pinning
    std::cout << "[Stage 9] Package Version Pinning & Protection...\n";
    TEST_ASSERT(mgr.pin("Microsoft.WindowsTerminal"), "Pin package");
    log.clear();
    TEST_ASSERT(mgr.upgrade("Microsoft.WindowsTerminal", log) == WINGET_INST_E_PACKAGE_PINNED, "Pinned upgrade blocked");
    TEST_ASSERT(mgr.unpin("Microsoft.WindowsTerminal"), "Unpin package");
    std::cout << "  [PASS] Version pinning protected package from unauthorized upgrades.\n";

    // 10. Uninstallation
    std::cout << "[Stage 10] Testing Safe Package Uninstallation...\n";
    log.clear();
    uint32_t countBefore = mgr.getInstalledCount();
    TEST_ASSERT(mgr.uninstall("Microsoft.WindowsTerminal", log) == WINGET_S_OK, "Uninstall Windows Terminal");
    TEST_ASSERT(mgr.getInstalledCount() == countBefore - 1, "Package count decremented");
    std::cout << "  [PASS] Package uninstalled cleanly.\n";

    // 11. C ABI Export Parity
    std::cout << "[Stage 11] Verifying Clean-Room C ABI Function Signatures...\n";
    void* pMgr = nullptr;
    TEST_ASSERT(WinGetCreatePackageManager(&pMgr) == WINGET_S_OK, "WinGetCreatePackageManager");
    TEST_ASSERT(pMgr != nullptr, "Pointer returned");

    uint32_t count = 0;
    TEST_ASSERT(WinGetGetInstalledCount(&count) == WINGET_S_OK, "WinGetGetInstalledCount");
    std::cout << "  [PASS] Win32 C ABI export functions callable with expected ABI semantics.\n";

    std::cout << "\n========================================================================\n"
              << "[+] ALL 11 TEST STAGES PASSED (100% SUCCESS)\n"
              << "========================================================================\n";
    return 0;
}
