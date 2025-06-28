# macrypto

**macrypto** is a lightweight, cross-platform cryptographic toolkit written in C.  
It supports macOS, Linux, and Windows, and provides essential cryptographic utilities such as AES encryption, Base64 encoding, and random number generation.

## ✨ Features

- 🔐 AES-128/256 encryption and decryption
- 🧮 Base64 encode/decode
- 🎲 Secure random number generation
- ⚙️ Command-line utility and library usage
- 💻 Cross-platform: macOS, Linux, Windows

## 📦 Build

### On Linux/macOS

```bash
make
```

### On Windows (MinGW)

```bash
make -f Makefile.win
```

> Or use Visual Studio project if provided.

## 🚀 Usage

Command-line example:

```bash
./macrypto aes -e -k mykey.txt -i input.txt -o output.enc
```

Library example (C):

```c
#include "macrypto.h"

macrypto_aes_encrypt(...);
```

## 📄 License

This project is licensed under the [MIT License](LICENSE).

## 🙋 Author

Developed by **Masato Ro**  
GitHub: [@masato-ro](https://github.com/masato-ro)
