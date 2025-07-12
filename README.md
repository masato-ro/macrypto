# Macrypt

**Macrypt** is a modern, cross-platform GUI and CLI tool for secure file encryption and decryption.
It supports **AES-256-CBC** and **GPG (GnuPG)** encryption methods and offers both graphical and command-line interfaces.

## Features

- AES-256 encryption and decryption (using OpenSSL)
- GPG encryption and decryption (via embedded GnuPG executable)
- SHA and MD5 hash utilities
- GUI built with Qt 6, cross-platform support
- **Modular design with separate controller classes (AESController, GPGController, HashController, KeyController)**
- Lightweight CLI and GUI versions (For CLI usage, refer to the built executable help or future documentation)
- Supports Windows, macOS, and Linux

## GUI Version

The GUI version of Macrypt supports file drag-and-drop, progress tracking, and log output.
It includes dedicated tabs for AES, GPG, and hashing tools.

## Build Instructions

### General Steps (Linux / macOS / Windows)

Make sure you have Qt 6.x and CMake installed.

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Configure the project using CMake (Release type is recommended)
#    -DCMAKE_BUILD_TYPE=Release enables optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. Build the project
cmake --build .

# After building, the executable will usually be located in 'build/bin/' (or 'build/Release/' etc.)
```

## CLI Usage

CLI functionality is integrated into the new build system.
Check the compiled executable (e.g., `macrypt.exe` or `macrypt`) and its help message for the latest CLI usage.

## Dependencies

- Qt 6.x
- OpenSSL 3.x
- GPG executable (bundled for portability)

## Encryption Methods

Macrypt supports two cryptographic approaches:

1. Native AES file encryption using OpenSSL. This is a straightforward symmetric encryption implementation.
2. GPG-based decryption of `.gpg` files. GPG internally uses AES or other symmetric ciphers as part of the OpenPGP standard.

## License

This project is licensed under the MIT License.

### Bundled or Used Libraries:

- Qt – for GUI (Qt Widgets)  
  License: LGPL v3

- OpenSSL – for cryptographic operations (e.g., AES), related files may be bundled in the `licenses/` folder or compiled binaries  
  License: Apache License 2.0

- GnuPG (gpg.exe) – for GPG decryption, bundled with the application  
  License: GPL-3.0-or-later

- libgcrypt – used by GnuPG  
  License: LGPL-2.1-or-later

### Additional Notes:

If you redistribute bundled binaries (e.g., `libcrypto-3.dll`, `gpg.exe`, etc.), you are responsible for complying with each library’s license terms.

## About The PenguinBay Software

Developed by The PenguinBay Software.  
GitHub: @masato-ro