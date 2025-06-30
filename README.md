# Macrypt

**Macrypt** is a modern cross-platform GUI and CLI tool for secure file encryption and decryption.  
It supports **AES-256-CBC** and **GPG (GnuPG)** encryption methods, offering both graphical and command-line interfaces.

## ✨ Features

- 🔐 AES-256 encryption and decryption (using OpenSSL)
- 📄 GPG encryption and decryption (via bundled GnuPG executable)
- 🧮 SHA and MD5 hashing utility
- 📦 GUI built with Qt 6 (cross-platform)
- ⚙️ Lightweight CLI and GUI versions available
- 💻 Runs on Windows, macOS, and Linux

## 🖼 GUI Version

The GUI version of Macrypt allows drag-and-drop file selection, progress tracking, and log output.  
It includes dedicated tabs for AES, GPG, and hashing utilities.

## 🔧 Build Instructions

### Linux / macOS

```bash
qmake
make
```

### Windows (MSVC)

```cmd
qmake -tp vc
nmake
```

Or open the `aescrypt_gui.pro` project file in **Qt Creator**.

## 🚀 CLI Usage

AES encryption from command-line:

```bash
./macrypt aes -e -k mykey.txt -i input.txt -o output.enc
```

Library usage in C:

```c
#include "aescrypt.h"

encrypt_file("input.txt", "output.enc", "your-password");
```

## 📦 Dependencies

- Qt 6.x
- OpenSSL 3.x
- GPG executable (bundled for portability)

## 📄 License

This project is licensed under the [MIT License](LICENSE).

Bundled libraries:

- **OpenSSL**: Apache License 2.0
- **Qt**: LGPL
- **GnuPG**: GPL-3.0-or-later
- **libgcrypt**: LGPL-2.1-or-later

## 👤 Author

Developed by **Masato Ro**  
GitHub: [@masato-ro](https://github.com/masato-ro)