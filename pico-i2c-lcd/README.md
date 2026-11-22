# Pico I²C LCD 1602

Raspberry Pi Pico（RP2040）で、PCF8574 互換の I²C 基板を備えた HD44780 互換の 16 桁 × 2 行 LCD を動かすサンプルです。ボード名や `hike-lang` を含む 5 ページのメッセージを、2 行ずつ中央寄せで 2 秒間表示し、繰り返します。

## 配線

| LCD 基板 | Raspberry Pi Pico |
|---|---|
| SDA | GP16（物理ピン 21） |
| SCL | GP17（物理ピン 22） |
| VCC | 3V3(OUT)（物理ピン 36、3.3 V 対応の LCD 基板を使用） |
| GND | GND（例として物理ピン 23） |

I²C0 を 100 kHz で使用します。LCD 基板のアドレスが異なる場合は [i2c-lcd.hike](i2c-lcd.hike) の `LcdAddress`（初期値 `0x27`）を変更してください。LCD 基板を 5 V で使う場合は SDA/SCL に双方向レベルシフタを入れます。Pico の GPIO に 5 V を入力しないでください。

## ビルドと書き込み

[QuickStart.md](../QuickStart.md) の手順で `hike-lang/hikec` をビルドします。ビルドには `clang`、`arm-none-eabi-gcc`、CMake、Ninja と、隣接する `pico-sdk/` が必要です。初回は picotool の取得にネットワーク接続が必要になる場合があります。

```bash
cd pico-i2c-lcd
make
```

生成される UF2 は `build/pico_lcd_1602_i2c.uf2` です。Pico の BOOTSEL ボタンを押しながら USB に接続し、マウントされた `RPI-RP2` ドライブにコピーするか、`make flash` を実行します。ドライブの場所は `make flash MOUNT=/path/to/mount` で指定できます。

L チカからコピーしたビルドディレクトリが残っている場合は、`make clean` で古い CMake キャッシュを消してからビルドします。

## 実装

- [main.hike](main.hike) がメッセージと表示順を定義します。
- [i2c-lcd.hike](i2c-lcd.hike) が 4 ビットモードへの初期化、データ送信、カーソル位置、消去、中央寄せを処理します。LCD 制御の参考にした C コードは [_example_main.c](_example_main.c) にあります。
- [lib/hike_shim.c](lib/hike_shim.c) が Pico SDK の I²C と待ち時間 API を Hike から呼べるようにします。[lib/hike_crt_main.c](lib/hike_crt_main.c) が Hike の `setup()` と `loop()` を呼び出します。
