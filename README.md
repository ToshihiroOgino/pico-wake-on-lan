# Pico Server

## 設定

picotool を sudo なしで実行する設定

下記内容で`/etc/udev/rules.d/99-pico.rules`を作成した後、`sudo udevadm control --reload-rules && sudo udevadm trigger`を実行する

```plaintext
# Pico 2w
# lsusb の結果が以下のような場合の設定
# BOOTSEL
# Bus 003 Device 003: ID 2e8a:000f Raspberry Pi RP2350 Boot
# Normal mode
# Bus 003 Device 015: ID 2e8a:0009 Raspberry Pi Pico
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000f", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0009", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0009", MODE="0666"

# Pico w
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0003", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000a", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000a", MODE="0666"
```

## 設定書き込み

XIP の空き領域に WiFi パスワードなどの設定を書き込むためのコマンド

```sh
picotool load -fx -o 0x10100000 -t bin config
```

## Time-Based One-Time Password

### 仕様

TOTP の仕様は [RFC 6238](https://datatracker.ietf.org/doc/html/rfc6238) に準拠する

- 生成インターバル: 30 秒
- ハッシュアルゴリズム: SHA-256
- シークレットキー: Base32 エンコードされた文字列
- トークン長: 6 桁

### OTP 認証方法

TCP で OTP だけを pico-server の IP アドレスに送信する

```sh
# echo + nc
echo -n "012345" | nc <pico-server-ip> <port>
# response: 012345_ok or 012345_ng
```

```sh
# .env ファイルが必要
python3 ./client/client.py
```
