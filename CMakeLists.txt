cmake_minimum_required(VERSION 3.16)
project(macrypt_gui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找 Qt 套件
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

# OpenSSL 路徑設定（依你本機位置修改）
include_directories("C:/OpenSSL-MSVC/include")
link_directories("C:/OpenSSL-MSVC/lib")

# 加入你的程式碼
add_executable(macrypt_gui WIN32
    src/main_gui.cpp
    src/mainwindow.cpp
    src/aescontroller.cpp
    src/aescrypt.cpp
    src/hashcontroller.cpp
    src/hashutil.cpp
    src/keycontroller.cpp
    src/keygen.cpp
    src/gpgcontroller.cpp
    src/cli_handler.cpp

    include/mainwindow.h
    include/aescrypt.h
    include/keygen.h
    include/hashutil.h
    include/gpgcontroller.h
    include/cli_handler.h

    src/mainwindow.ui
    resources.qrc
)

target_include_directories(macrypt_gui PRIVATE include)

target_link_libraries(macrypt_gui PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    libssl libcrypto
)

# Windows 資源檔（如果你還有 app.rc）
if (WIN32)
    target_sources(macrypt_gui PRIVATE app.rc)
endif()

# CLI target
add_executable(macrypt_cli
    src/main_cli.cpp
    src/cli_handler.cpp
    src/aescrypt.cpp
    src/aescontroller.cpp
    src/hashutil.cpp
    src/hashcontroller.cpp
    src/keygen.cpp
    src/keycontroller.cpp
    src/gpgcontroller.cpp

    include/aescrypt.h
    include/keygen.h
    include/hashutil.h
    include/gpgcontroller.h
    include/cli_handler.h
)

target_include_directories(macrypt_cli PRIVATE include)

target_link_libraries(macrypt_cli PRIVATE
    Qt6::Core
    libssl libcrypto
)

## 測試配置
# 為測試檔案創建一個獨立的可執行檔
add_executable(macrypt_gui_test
    src/test_functions.cpp
    src/hashcontroller.cpp
    src/hashutil.cpp
    src/aescrypt.cpp
    src/keygen.cpp
    src/gpgcontroller.cpp
    src/cli_handler.cpp

    include/aescrypt.h
    include/keygen.h
    include/gpgcontroller.h
    include/cli_handler.h
)

# 設定測試可執行檔的 include 目錄
# 讓它可以找到 hashcontroller.h 以及其他潛在的頭文件
target_include_directories(macrypt_gui_test PRIVATE
    include # 包含你專案的頭文件，例如 hashcontroller.h
)

# 連結測試所需的 Qt 模組和其他函式庫
target_link_libraries(macrypt_gui_test PRIVATE
    Qt6::Test # QtTest 框架
    Qt6::Core # 測試通常也需要 Qt Core

    # hashcontroller 可能使用 OpenSSL，所以測試也需要連結
    "C:/OpenSSL-MSVC/lib/libssl.lib"
    "C:/OpenSSL-MSVC/lib/libcrypto.lib"
)

# 啟用 CTest，並添加測試案例
enable_testing()
add_test(NAME HashControllerTest COMMAND macrypt_gui_test)

# 設定編譯選項
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Wdeprecated-declarations)
endif()