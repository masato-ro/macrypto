
# 🛠️ Macrypt 開發進度紀錄

## ✅ 已完成功能

- ✔️ OpenSSL 金鑰產生（私鑰、公鑰）
- ✔️ 支援 OpenSSH 格式公鑰，自動轉換並儲存為 `.pub`
- ✔️ 可自訂 OpenSSH 公鑰備註（Comment），留空則自動使用機器名稱
- ✔️ 產生後顯示私鑰、公鑰、OpenSSH 公鑰路徑於 log
- ✔️ 公鑰路徑與檔名會依私鑰自動對應命名
- ✔️ 加入複製公鑰至剪貼簿的按鈕
- ✔️ comment 欄位 placeholder 提示使用者留空會使用預設主機名稱
- ✔️ Menu 選單整合各頁籤（Tab）
- ✔️ Menu action 支援快捷鍵（Ctrl+1~4）
- ✔️ 選單與 tabWidget 同步 checked 狀態（highlight 哪個頁籤）
- [ ] log 區域優化：
  - ✔️ 加入「清除 log」按鈕

---

## 📝 TODO 功能

- [ ] ⚠️ 移除 OpenSSL 3.0 deprecated API（`EVP_PKEY_get1_RSA`、`RSA_free` 等）
- [ ] Hash Digest：支援讀取檔案並計算雜湊值
- [ ] GPG 解密：強化錯誤處理與進度提示
- [ ] log 區域優化：
  - [ ] 長路徑自動省略中間顯示（可滑鼠 hover 顯示完整）
- [ ] 增加使用者設定儲存功能（如預設金鑰長度、預設輸出目錄等）
- [ ] 語言切換支援（中文、日文、英文）

---

## 🔧 技術細節

- 使用 Qt 6.9.1
- 使用 OpenSSL 3.0 API（需過渡至非 deprecated 寫法）
- 支援 RSA 1024/2048/4096 位元
- 支援 base64 + comment 格式輸出為 OpenSSH key

---

> 建議每日開工前先複習此清單，規劃當日目標，善用 `git commit` 留下紀錄 💪
