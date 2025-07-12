#include <QtTest>
#include <QFile> // 新增，用於檔案操作
#include <QDir> // 新增，用於檔案和目錄操作
#include <QCoreApplication> // 新增，用於處理事件以便進度條更新
#include <QDebug>
#include <functional> // 新增，用於 std::function
#include <QStandardPaths> // 新增，用於獲取臨時檔案路徑
#include <QProcess>
#include <QCryptographicHash> // 新增，用於加密雜湊計算

#include "hashutil.h"
#include "hashcontroller.h"
#include "aescrypt.h"
#include "keygen.h"

struct evp_pkey_st; // 前置宣告，避免包含 OpenSSL 標頭檔案
typedef struct evp_pkey_st EVP_PKEY; // 定義 EVP_PKEY 類型

class TestHashUtilities : public QObject
{
    Q_OBJECT

private:
    QString m_tempFilePath; // 儲存臨時檔案的路徑 (for Hash tests)
    QString m_aesTestFileInPath; // 用於 AES 加密的輸入測試檔案路徑
    QString m_aesTestFileEncPath; // 用於 AES 加密的輸出(加密後)測試檔案路徑
    QString m_aesTestFileDecPath; // 用於 AES 解密後的輸出測試檔案路徑

    QString m_privKeyPath; // 私鑰檔案路徑
    QString m_pubKeyPath;  // 公鑰檔案路徑
    QString m_tempDir;     // 臨時金鑰檔案的目錄

private slots:
    // 初始化測試案例，例如創建臨時檔案
    void initTestCase() {
        // 確保應用程式實例存在，以便 QCoreApplication::processEvents() 能運作
        if (!QCoreApplication::instance()) {
            static int argc = 0;
            new QCoreApplication(argc, nullptr);
        }
        m_tempFilePath = "test_temp_file_for_hash.tmp";

        m_aesTestFileInPath = "aes_test_input.tmp";
        m_aesTestFileEncPath = "aes_test_encrypted.tmp";
        m_aesTestFileDecPath = "aes_test_decrypted.tmp";

        m_tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + "macrypt_keygen_tests";
        QDir().mkpath(m_tempDir); // 確保臨時目錄存在

        m_privKeyPath = m_tempDir + QDir::separator() + "test_private_key.pem";
        m_pubKeyPath = m_tempDir + QDir::separator() + "test_public_key.pem";
    }

    void testRunHashFromText_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<HashAlgorithm>("alg");
        QTest::addColumn<bool>("expectEmpty");

        QTest::newRow("empty text") << "" << HashAlgorithm::MD5 << true;
        QTest::newRow("simple text") << "abc" << HashAlgorithm::MD5 << false;
    }

    void testRunHashFromText() {
        QFETCH(QString, input);
        QFETCH(HashAlgorithm, alg);
        QFETCH(bool, expectEmpty);

        QString result = HashController::runHashFromText(input, alg);
        if (expectEmpty) {
            QVERIFY(result.isEmpty());
        } else {
            QVERIFY(!result.isEmpty());
        }
    }

    void testComputeHashFromFile_data() {
        QTest::addColumn<QString>("inputContent");
        QTest::addColumn<HashAlgorithm>("alg");
        QTest::addColumn<QString>("expectedHash");
        QTest::addColumn<bool>("expectSuccess");
        QTest::addColumn<QList<int>>("expectedProgressSequence");

        // 案例 1: 空檔案
        // MD5 for empty string: d41d8cd98f00b204e9800998ecf8427e
        // SHA256 for empty string: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
        QTest::newRow("empty file MD5")    << "" << HashAlgorithm::MD5    << "d41d8cd98f00b204e9800998ecf8427e" << true << QList<int>({100});
        QTest::newRow("empty file SHA256") << "" << HashAlgorithm::SHA256 << "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" << true << QList<int>({100});

        // 案例 2: 小檔案
        // MD5 for "hello": 5d41402abc4b2a76b9719d911017c592
        // SHA256 for "world": cb0c19b089c1e7d362fb7b6a127814421d0a5e821d3f4b669b7e7161b4028c77
        QTest::newRow("small file MD5")    << "hello" << HashAlgorithm::MD5    << "5d41402abc4b2a76b9719d911017c592" << true << QList<int>({100});
        QTest::newRow("small file SHA256") << "world" << HashAlgorithm::SHA256 << "486ea46224d1bb4fb680f34f7c9ad96a8f24ec88be73ea8e5a6c65260e9cb8a7" << true << QList<int>({100});

        // 案例 3: 多次讀取的大檔案 (10KB)
        QString largeContent(1024 * 10, 'A'); // 10KB 內容
        QList<int> largeFileProgress;
        largeFileProgress << 40 << 80 << 100;
        // *** 修正這裡的 MD5 預期值 ***
        QTest::newRow("large file MD5") << largeContent << HashAlgorithm::MD5 << "b2a3affdfa9805c917df791087ac93d1" << true << largeFileProgress;

        // 案例 4: 不存在的檔案
        QTest::newRow("non-existent file") << QString() << HashAlgorithm::MD5 << QString() << false << QList<int>({});
    }

    void testComputeHashFromFile() {
        QFETCH(QString, inputContent);
        QFETCH(HashAlgorithm, alg);
        QFETCH(QString, expectedHash);
        QFETCH(bool, expectSuccess);
        QFETCH(QList<int>, expectedProgressSequence);

        // 1. 設定測試檔案路徑
        // 這裡是關鍵修正：直接使用 m_tempFilePath 作為要操作的檔案名稱
        QString currentTestFilePath = m_tempFilePath;

        // 2. 建立或準備臨時檔案
        // 對於「不存在的檔案」案例，我們不需要建立檔案
        if (inputContent.isEmpty() && expectSuccess) { // 這是為 "empty file" 案例建立空檔案
            QFile emptyFile(currentTestFilePath);
            if (!emptyFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                QFAIL("無法建立空的測試檔案"); // 錯誤訊息更明確
            }
            emptyFile.close();
        } else if (!inputContent.isEmpty()){ // 對於有內容的檔案
            QFile file(currentTestFilePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QFAIL("無法建立測試檔案");
            }
            file.write(inputContent.toUtf8());
            file.close();
        }

        // 3. 準備進度回呼的監聽器
        QList<int> actualProgress;
        auto progressCb = [&](int percent) {
            actualProgress.append(percent);
            QCoreApplication::processEvents();
        };

        // 4. 執行測試
        QString actualHash;
        if (expectedProgressSequence.isEmpty()) { // 這是 "non-existent file" 案例
            // 對於不存在的檔案，我們故意給一個不可能的路徑來觸發失敗
            actualHash = HashUtil::computeHashFromFile("this_file_really_should_not_exist_xyz.txt", alg, progressCb);
        } else {
            actualHash = HashUtil::computeHashFromFile(currentTestFilePath, alg, progressCb);
        }

        // 5. 驗證結果
        if (expectSuccess) {
            QVERIFY2(!actualHash.isEmpty(), "實際雜湊值不應為空");
            QCOMPARE(actualHash, expectedHash);
        } else {
            QVERIFY2(actualHash.isEmpty(), "實際雜湊值應該為空，因為檔案不存在或無法讀取");
        }

        // 6. 驗證進度回呼序列
        if (expectSuccess) {
            QVERIFY2(!actualProgress.isEmpty(), "成功的測試應該至少收到一個進度回呼 (100%)");
            QCOMPARE(actualProgress.last(), 100); // 確保最終進度是 100%

            // 對於有中間進度的情況，檢查是否符合預期
            if (expectedProgressSequence.size() > 1) { // 如果預期進度序列包含多個點
                // 檢查實際進度是否單調遞增
                for (int i = 1; i < actualProgress.size(); ++i) {
                    QVERIFY2(actualProgress.at(i) >= actualProgress.at(i-1), "進度回呼應該單調遞增");
                }
                // 簡單檢查：預期點位是否被達到過
                for (int expectedP : expectedProgressSequence) {
                    bool found = false;
                    for (int actualP : actualProgress) {
                        if (actualP >= expectedP) {
                            found = true;
                            break;
                        }
                    }
                    QVERIFY2(found, QString("預期進度 %1 未達到").arg(expectedP).toUtf8().constData());
                }
            }
        } else { // 對於失敗的案例
            QVERIFY2(actualProgress.isEmpty(), "失敗的測試不應有進度回呼");
        }

        // 7. 清理臨時檔案
        QFile::remove(currentTestFilePath);
    }

    void testAESCryptFile_data() {
        QTest::addColumn<QString>("inputContent");
        QTest::addColumn<QString>("password");
        QTest::addColumn<bool>("expectEncryptSuccess");
        QTest::addColumn<bool>("expectDecryptSuccess"); // 解密是否成功 (用對密碼)
        QTest::addColumn<QString>("wrongPassword"); // 用於測試密碼錯誤
        QTest::addColumn<bool>("expectWrongPasswordFail"); // 用錯密碼解密是否失敗
        QTest::addColumn<QList<int>>("expectedProgressSequence");

        // 案例 1: 空檔案
        QTest::newRow("empty file")
            << "" << "pass123" << true << true << "wrongpass" << true << QList<int>({100});

        // 案例 2: 小檔案 (單一區塊)
        QTest::newRow("small file")
            << "Hello, AES encryption!" << "mysecretpass" << true << true << "wrong_pass" << true << QList<int>({100});

        // 案例 3: 中等檔案 (多個區塊，觸發進度回呼)
        QString mediumContent(50000, 'B'); // 約 50KB
        QList<int> mediumProgress;
        mediumProgress << 0 << 20 << 40 << 60 << 80 << 100; // 預期進度點
        QTest::newRow("medium file")
            << mediumContent << "strong@pass123" << true << true << "another_wrong" << true << mediumProgress;

        // 案例 4: 非常大的檔案 (您可以根據需要調整大小)
        QString largeContent(1024 * 1024, 'C'); // 1MB
        QList<int> largeProgress;
        largeProgress << 0 << 10 << 20 << 30 << 40 << 50 << 60 << 70 << 80 << 90 << 100;
        QTest::newRow("large file")
            << largeContent << "supersecretkey" << true << true << "incorrect" << true << largeProgress;
    }

    void testAESCryptFile() {
        QFETCH(QString, inputContent);
        QFETCH(QString, password);
        QFETCH(bool, expectEncryptSuccess);
        QFETCH(bool, expectDecryptSuccess);
        QFETCH(QString, wrongPassword);
        QFETCH(bool, expectWrongPasswordFail);
        QFETCH(QList<int>, expectedProgressSequence);

        // 1. 寫入輸入檔案
        // 使用正確初始化的 AES 測試檔案路徑
        QFile inFile(m_aesTestFileInPath);
        if (!inFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QFAIL("無法建立輸入測試檔案"); // 錯誤訊息更明確
        }
        inFile.write(inputContent.toUtf8());
        inFile.close();

        // 2. 加密測試
        AESCrypt aesCrypt;
        QList<int> actualEncryptProgress;
        auto encryptProgressCb = [&](int percent) {
            actualEncryptProgress.append(percent);
            QCoreApplication::processEvents();
        };

        // 使用正確初始化的 AES 測試檔案路徑
        bool encryptResult = aesCrypt.encryptFile(m_aesTestFileInPath, m_aesTestFileEncPath, password, encryptProgressCb);

        QCOMPARE(encryptResult, expectEncryptSuccess);
        if (expectEncryptSuccess) {
            QVERIFY(QFile(m_aesTestFileEncPath).exists());
            // 對於空檔案，加密後會有鹽值和填充，所以大小會大於0
            // QVERIFY(QFile(m_aesTestFileEncPath).size() > 0 || inputContent.isEmpty());
            QVERIFY(QFile(m_aesTestFileEncPath).size() > 0); // 加密後的檔案大小至少包含 IV/Salt

            if (!expectedProgressSequence.isEmpty()) {
                QVERIFY(!actualEncryptProgress.isEmpty());
                QCOMPARE(actualEncryptProgress.last(), 100);
            }
        }

        // 3. 解密測試 (使用正確密碼)
        if (expectEncryptSuccess) { // 只有加密成功才能進行解密測試
            QList<int> actualDecryptProgress;
            auto decryptProgressCb = [&](int percent) {
                actualDecryptProgress.append(percent);
                QCoreApplication::processEvents();
            };

            // 使用正確初始化的 AES 測試檔案路徑
            bool decryptResult = aesCrypt.decryptFile(m_aesTestFileEncPath, m_aesTestFileDecPath, password, decryptProgressCb);

            QCOMPARE(decryptResult, expectDecryptSuccess);
            if (expectDecryptSuccess) {
                QVERIFY(QFile(m_aesTestFileDecPath).exists());
                QFile decryptedFile(m_aesTestFileDecPath);
                if (!decryptedFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QFAIL("無法開啟解密後的檔案進行驗證");
                }
                QByteArray decryptedContent = decryptedFile.readAll();
                decryptedFile.close();

                QCOMPARE(decryptedContent, inputContent.toUtf8()); // 驗證內容是否一致

                if (!expectedProgressSequence.isEmpty()) {
                    QVERIFY(!actualDecryptProgress.isEmpty());
                    QCOMPARE(actualDecryptProgress.last(), 100);
                }
            }
        }

        // 4. 解密測試 (使用錯誤密碼)
        if (expectEncryptSuccess && expectWrongPasswordFail && !wrongPassword.isEmpty()) {
            QFile::remove(m_aesTestFileDecPath); // 清理上次的解密結果
            // 使用正確初始化的 AES 測試檔案路徑
            bool wrongPassDecryptResult = aesCrypt.decryptFile(m_aesTestFileEncPath, m_aesTestFileDecPath, wrongPassword);
            QVERIFY(!wrongPassDecryptResult); // 應該會失敗
        }

        // 5. 清理臨時檔案 (單個測試結束後清理)
        QFile::remove(m_aesTestFileInPath);
        QFile::remove(m_aesTestFileEncPath);
        QFile::remove(m_aesTestFileDecPath);
    }

    void testAESCryptFile_NonExistentInput() {
        AESCrypt aesCrypt;
        // 確保輸入檔案不存在
        QString nonExistentPath = "non_existent_input_for_aes.tmp"; // 為這個測試使用一個新的臨時名稱
        QFile::remove(nonExistentPath);

        // 使用正確初始化的 AES 測試檔案路徑作為輸出
        bool result = aesCrypt.encryptFile(nonExistentPath, m_aesTestFileEncPath, "password");
        QVERIFY(!result); // 應該加密失敗
        QVERIFY(!QFile(m_aesTestFileEncPath).exists()); // 不應該產生輸出檔案

        // 清理可能因錯誤創建的輸出檔案（儘管預期不會創建）
        QFile::remove(m_aesTestFileEncPath);
    }

    void testGenerateRSAKeyPair_data() {
        QTest::addColumn<int>("bits");
        QTest::addColumn<bool>("expectSuccess");

        QTest::newRow("RSA 1024 bits") << 1024 << true;
        QTest::newRow("RSA 2048 bits") << 2048 << true;
        QTest::newRow("RSA 4096 bits") << 4096 << true;
        // OpenSSL RSA 金鑰的最小建議位元數通常是 1024 位元。測試一個較低的值來檢查是否會失敗。
        QTest::newRow("RSA invalid bits (too low)") << 512 << false;
        QTest::newRow("RSA large bits (valid)") << 8192 << true; // 很大，但通常支援
    }

    void testGenerateRSAKeyPair() {
        QFETCH(int, bits);
        QFETCH(bool, expectSuccess);

        // 確保檔案不存在，避免上次運行殘留
        QFile::remove(m_privKeyPath);
        QFile::remove(m_pubKeyPath);

        bool result = generateRSAKeyPair(bits, m_privKeyPath, m_pubKeyPath);

        QCOMPARE(result, expectSuccess);

        if (expectSuccess) {
            QVERIFY(QFile(m_privKeyPath).exists());
            QVERIFY(QFile(m_pubKeyPath).exists());
            QVERIFY(QFile(m_privKeyPath).size() > 0);
            QVERIFY(QFile(m_pubKeyPath).size() > 0);

            // 嘗試載入私鑰以驗證其有效性和位元數
            EVP_PKEY* loadedPkey = loadPrivateKeyFromFile(m_privKeyPath);
            QVERIFY2(loadedPkey != nullptr, "生成的私鑰應該可載入。");
            if (loadedPkey) {
                QCOMPARE(EVP_PKEY_bits(loadedPkey), bits); // 驗證位元數
                EVP_PKEY_free(loadedPkey);
            }
        } else {
            // 如果預期會失敗，則確保沒有檔案被建立
            QVERIFY(!QFile(m_privKeyPath).exists());
            QVERIFY(!QFile(m_pubKeyPath).exists());
        }
    }

    void testLoadPrivateKeyFromFile() {
        // 首先，生成一個有效的金鑰對來測試成功載入
        int bits = 2048;
        bool generateResult = generateRSAKeyPair(bits, m_privKeyPath, m_pubKeyPath);
        QVERIFY(generateResult);

        // 測試成功載入生成的私鑰
        EVP_PKEY* loadedPkey = loadPrivateKeyFromFile(m_privKeyPath);
        QVERIFY2(loadedPkey != nullptr, "應該能夠成功載入有效的私鑰。");
        if (loadedPkey) {
            QCOMPARE(EVP_PKEY_bits(loadedPkey), bits);
            EVP_PKEY_free(loadedPkey);
        }

        // 測試載入不存在的檔案
        EVP_PKEY* nonExistentPkey = loadPrivateKeyFromFile("non_existent_key_for_load.pem");
        QVERIFY2(nonExistentPkey == nullptr, "載入不存在的檔案應該失敗。");

        // 測試載入空檔案
        QString emptyFilePath = m_tempDir + QDir::separator() + "empty_key.pem";
        QFile emptyFile(emptyFilePath);
        QVERIFY(emptyFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        emptyFile.close();
        EVP_PKEY* emptyFilePkey = loadPrivateKeyFromFile(emptyFilePath);
        QVERIFY2(emptyFilePkey == nullptr, "載入空檔案應該失敗。");
        QFile::remove(emptyFilePath);

        // 測試載入無效格式的檔案（例如，只寫入一些隨機文字）
        QString invalidFormatPath = m_tempDir + QDir::separator() + "invalid_format_key.pem";
        QFile invalidFormatFile(invalidFormatPath);
        QVERIFY(invalidFormatFile.open(QIODevice::WriteOnly | QIODevice::Text));
        invalidFormatFile.write("這不是一個有效的 PEM 金鑰。");
        invalidFormatFile.close();
        EVP_PKEY* invalidFormatPkey = loadPrivateKeyFromFile(invalidFormatPath);
        QVERIFY2(invalidFormatPkey == nullptr, "載入無效格式檔案應該失敗。");
        QFile::remove(invalidFormatPath);
    }

    void testGenerateOpenSSHPublicKey_data() {
        QTest::addColumn<QString>("comment");
        QTest::addColumn<bool>("expectSuccess");

        QTest::newRow("no comment") << "" << true;
        QTest::newRow("simple comment") << "test@example.com" << true;
        QTest::newRow("complex comment") << "User Key (generated by app)" << true;
    }

    void testGenerateOpenSSHPublicKey() {
        QFETCH(QString, comment);
        QFETCH(bool, expectSuccess);

        // 確保有私鑰可以生成公鑰
        int bits = 2048;
        // 清理並重新生成金鑰，以確保每個測試案例的隔離性
        QFile::remove(m_privKeyPath);
        QFile::remove(m_pubKeyPath);

        bool generateResult = generateRSAKeyPair(bits, m_privKeyPath, m_pubKeyPath);
        QVERIFY(generateResult);

        EVP_PKEY* pkey = loadPrivateKeyFromFile(m_privKeyPath);
        QVERIFY(pkey != nullptr);

        QString sshPublicKey = generateOpenSSHPublicKey(pkey, comment);

        if (expectSuccess) {
            QVERIFY(!sshPublicKey.isEmpty());
            QVERIFY(sshPublicKey.startsWith("ssh-rsa "));

            // 檢查註解是否正確附加
            // 由於 OpenSSH 的公鑰格式中，註解是可選的，並且在 'ssh-rsa <base64-blob> <comment>' 結構中。
            // 如果 comment 為空，它就不會被包含在最終字串中。
            if (!comment.isEmpty()) {
                QVERIFY2(sshPublicKey.endsWith(" " + comment),
                         QString("生成的金鑰 '%1' 未以預期的註解 '%2' 結尾").arg(sshPublicKey).arg(comment).toUtf8().constData());
            } else {
                // 如果註解為空，確保最終字串中 Base64 後面沒有多餘的空格
                QStringList parts = sshPublicKey.split(' ');
                QCOMPARE(parts.size(), 2); // 應該只有 "ssh-rsa" 和 Base64 blob
            }

        } else {
            // 如果預期會失敗（例如，非 RSA 金鑰，儘管這裡沒有明確測試），結果應該為空
            QVERIFY(sshPublicKey.isEmpty());
        }

        if (pkey) { // 釋放金鑰資源
            EVP_PKEY_free(pkey);
        }
    }   

    void cleanupTestCase() {
        // 這裡可以做一些清理工作，例如確保所有臨時檔案都已被移除
        QFile::remove(m_tempFilePath);
        QFile::remove(m_aesTestFileInPath);
        QFile::remove(m_aesTestFileEncPath);
        QFile::remove(m_aesTestFileDecPath);

        // --- 新增：KeyGen 測試的清理 ---
        QFile::remove(m_privKeyPath);
        QFile::remove(m_pubKeyPath);
        // 移除臨時目錄及其所有內容
        if (QDir(m_tempDir).exists()) {
             QDir(m_tempDir).removeRecursively();
        }
    }
};

QTEST_MAIN(TestHashUtilities)
#include "test_functions.moc"