# pico-blink

Raspberry Pi Pico (RP2040) の内蔵LED(GPIO25)を点滅させるLチカサンプルです。ロジック本体を [blink.hike](blink.hike) というHikeコードで書き、pico-sdkのビルドに組み込んで実機用の `.uf2` ファイルを生成します。

## 仕組み

```
blink.hike ──hikec emit-ir──▶ blink.ll ──clang -c (thumbv6m-none-eabi)──▶ blink.o
                                                                              │
lib/main.c + lib/hike_shim.c (arm-none-eabi-gcc) ────────────────────────────┤
                                                                              ▼
                                    pico-sdkのリンカスクリプト・起動コード等 ──▶ blink.elf → blink.uf2
```

- [blink.hike](blink.hike): Arduinoの`setup()`/`loop()`に似た構成。GPIO初期化を`setup()`、点滅処理を`loop()`に書く
- [lib/hike_shim.c](lib/hike_shim.c): pico-sdkの`gpio_init`/`gpio_put`/`sleep_ms`などをHike側から呼びやすい`uint32`だけの薄いラッパーにしたもの(HikeとCの`bool`表現の違いに配慮するため)
- [lib/main.c](lib/main.c): pico-sdkの`main()`から`setup()`を1回、その後`loop()`を無限に呼ぶだけ
- [lib/CMakeLists.txt](lib/CMakeLists.txt): `hikec`→`clang -c`でblink.oを作るカスタムビルドステップを、通常のpico-sdkビルドに追加している
- [lib/pico_sdk_import.cmake](lib/pico_sdk_import.cmake): pico-sdk公式の取り込み用スクリプト(そのままコピー)

`blink.hike`自体はGPIOレジスタを直接叩いているわけではなく、pico-sdkの既存API(`gpio_init`等)をC-ABI extern関数として呼び出しているだけです。ブートローダ・リンカスクリプト・起動コード・UF2変換は全てpico-sdkに任せています。

対象ボードはデフォルトの `pico`(無印Raspberry Pi Pico、RP2040)です。LEDピンはボード定義から決まる`PICO_DEFAULT_LED_PIN`(=25)を使っています。

## 前提条件

- このリポジトリと同じ階層に `pico-sdk/` があること(`../pico-sdk`)
- `arm-none-eabi-gcc` / `cmake` / `ninja` (pico-sdkのビルド用)
- `clang` (`blink.hike`から生成したLLVM IRをThumbオブジェクトにする用)
- `hike-lang/hikec` がビルド済みであること( [../QuickStart.md](../QuickStart.md) 参照)
- 初回ビルド時のみ、`picotool`をソースから取得・ビルドするためネットワーク接続が必要

## Makeターゲット

```bash
make          # 設定 + ビルド。build/blink.uf2 まで生成する
make clean    # build/ ディレクトリを削除
make flash    # BOOTSELモードで接続されたPicoにblink.uf2を書き込む
```

- `make` は内部で `cmake -S lib -B build -G Ninja` (初回のみ) → `cmake --build build` を実行します。CMake/Ninjaはpico-sdkのボード設定・ブートローダのチェックサム生成に使われているため裏で動いていますが、日常の操作は`make`だけで完結します。
- `make flash` は `/run/media/$USER/RPI-RP2` または `/media/$USER/RPI-RP2` にマウントされたドライブへ自動でコピーします。マウント先が異なる場合は `make flash MOUNT=/path/to/mount` のように指定してください。
- PicoをBOOTSELモード(BOOTSELボタンを押しながらUSB接続)にすると、PCからはUSBメモリとして認識されます。`make flash`が使えない環境では、生成された `build/blink.uf2` をそのドライブに手動でコピーしても書き込めます。

## Hikeコード側の注意点

`blink.hike`で宣言するextern関数名は、Hikeコンパイラが常に埋め込む組み込みランタイムの予約シンボル名(`hike_sleep_ms`, `hike_now_ns`, `hike_thread_spawn`, `hike_event_*`など、`hike_`プレフィックスの名前)と衝突させないでください。衝突すると意図した関数ではなく、ランタイム内蔵の実装(このターゲットには存在しないOS機能を呼ぶもの)にリンクされてしまいます。本サンプルでは `pico_` プレフィックスを使っています。
