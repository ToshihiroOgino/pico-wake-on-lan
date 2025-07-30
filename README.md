# Pico2 Server

## 設定

picotool を sudo なしで実行する設定

下記内容で`/etc/udev/rules.d/99-pico.rules`を作成した後、`sudo udevadm control --reload-rules &&  sudo udevadm trigger`を実行する

```plaintext
# lsusb の結果が以下のような場合の設定
# BOOTSEL
# Bus 003 Device 003: ID 2e8a:000f Raspberry Pi RP2350 Boot
# Normal mode
# Bus 003 Device 015: ID 2e8a:0009 Raspberry Pi Pico

SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000f", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0009", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0009", MODE="0666"
```
