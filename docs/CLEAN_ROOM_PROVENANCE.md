# Clean-Room Provenance & Interoperability Architecture

> **Policy Statement**: This repository is a clean-room, zero-telemetry implementation of the Windows Package Manager interface, manifest parser, dependency DAG solver, and command-line execution broker authored in modern ISO C++23.

---

## 1. Specification Reference
This project was implemented exclusively from first principles using publicly published, open specifications:
- **YAML Manifest Specification Schema v1.6.0** published by the [`microsoft/winget-pkgs`](https://github.com/microsoft/winget-pkgs) and [`microsoft/winget-cli`](https://github.com/microsoft/winget-cli) open-source repositories.
- **NIST FIPS 180-4 Secure Hash Standard (SHS)** for freestanding SHA-256 cryptographic digest computation.
- **Win32 Metadata** (`microsoft/win32metadata`) for C ABI declarations (`AppInstaller.dll`).

## 2. Legal Precedent & Fair Use
Under United States copyright law and the landmark Supreme Court decision in *Google LLC v. Oracle America, Inc.* (593 U.S. ___ 2021), the reimplementation of functional declaring interfaces, API method signatures, and structured manifest schemas for binary interoperability is protected fair use.

No proprietary, leaked, or confidential Microsoft source code was viewed, decompiled, or used in the creation of this software.

## 3. Zero-Telemetry Policy
Unlike upstream enterprise package managers that bundle telemetry probes, activity correlation vectors, and remote analytics beacons, this engine contains zero telemetry hooks by design.
