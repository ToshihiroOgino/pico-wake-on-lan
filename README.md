# Pico2 Server

## 設定

picotool を sudo なしで実行する

`lsusb`の結果が`Bus 003 Device 003: ID 2e8a:000f Raspberry Pi RP2350 Boot`の時の例

`/etc/udev/rules.d/99-pico.rules`に以下を追加したあと、`sudo udevadm control --reload-rules &&  sudo udevadm trigger`を実行する

```plaintext
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000f", MODE="0666"
```
