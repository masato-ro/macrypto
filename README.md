# Macrypt

**Macrypt** is a modern cross-platform GUI and CLI tool for secure file encryption and decryption.  
It supports **AES-256-CBC** and **GPG (GnuPG)** encryption methods, offering both graphical and command-line interfaces.

## Features

- AES-256 encryption and decryption (using OpenSSL)  
- GPG encryption and decryption (via bundled GnuPG executable)  
- SHA and MD5 hashing utility  
- GUI built with Qt 6 (cross-platform)  
- Lightweight CLI and GUI versions available  
- Runs on Windows, macOS, and Linux

## GUI Version

The GUI version of Macrypt allows drag-and-drop file selection, progress tracking, and log output.  
It includes dedicated tabs for AES, GPG, and hashing utilities.

## Build Instructions

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

## CLI Usage

AES encryption from command-line:

```bash
aescrypt_gui.exe encrypt <input> <output> <password>
aescrypt_gui.exe decrypt <input> <output> [password]
```

- `<input>`: input file path
- `<output>`: output file path
- `<password>`: required for AES encryption/decryption; optional for GPG decryption (omit to use pinentry)

Examples:

```bash
aescrypt_gui.exe encrypt plain.txt encrypted.aes mypassword
aescrypt_gui.exe decrypt encrypted.aes decrypted.txt mypassword
aescrypt_gui.exe decrypt secret.gpg decrypted.txt
```

## Dependencies

- Qt 6.x
- OpenSSL 3.x
- GPG executable (bundled for portability)


## Notes on Encryption Methods

Macrypt supports two cryptographic modes:

- Native AES file encryption using OpenSSL. This is a direct symmetric encryption implementation.
- Decryption of .gpg files via GPG, which internally uses AES or other symmetric ciphers as part of the OpenPGP standard.


## License

This project is licensed under the [MIT License](LICENSE).

### Bundled or Used Libraries:

- **Qt** – Used for GUI (Qt Widgets)  
  License: [LGPL v3](https://www.qt.io/licensing)

- **OpenSSL** – Used for cryptographic operations (e.g., RSA, AES), headers may be bundled  
  License: [Apache License 2.0](https://www.openssl.org/source/license.html)  
  If headers are bundled, see `include/openssl/` and `licenses/openssl_apache-2.0.txt`

- **GnuPG (gpg.exe)** – Used for GPG decryption  
  License: [GPL-3.0-or-later](https://www.gnu.org/licenses/gpl-3.0.html)

- **libgcrypt** – Used by GnuPG  
  License: [LGPL-2.1-or-later](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)

---

### Additional Notes:

You are responsible for complying with each library’s license if you redistribute the bundled binaries (e.g., `libcrypto-3.dll`, `gpg.exe`, etc.).


## About The PenguinBay Software

Developed by **The PenguinBay Software**.
GitHub: [@masato-ro](https://github.com/masato-ro)