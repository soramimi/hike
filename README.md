# hike workspace

[Hike言語](https://github.com/kanryu/hike-lang) を試すための作業用ワークスペースです。コンパイラ本体のクローンに加え、Raspberry Pi Pico (RP2040) 向けにHikeでLチカを動かす実験 ([pico-blink/](pico-blink/)) を含んでいます。

## 構成

```
hike-lang/    Hike言語コンパイラ (hikec) のクローン
pico-sdk/     Raspberry Pi Pico SDK (pico-blink のビルドに使用)
pico-blink/   RP2040向けLチカサンプル(一部をHikeで記述)
QuickStart.md hikecのビルド〜Hello World実行までの手順
```

## Hike言語とは

Hikeは、Go風の構文を持ちながらGCを持たない軽量なシステムプログラミング言語です。

- LLVM IR または WebAssembly(WAT)に変換され、ネイティブ実行ファイル・C-ABI準拠の共有ライブラリ(`.dll`/`.so`)・Wasmモジュールを生成できる
- GCなし、常駐ランタイムなし。ランタイムサポートは使用した機能に応じて必要な分だけリンクされる
- 型推論(`:=`)、多値返却、スライス、構造体、ジェネリクスなどGo譲りの機能を持つ
- `-g`フラグでDWARFデバッグ情報を生成でき、GDB/LLDB(ネイティブ)やChrome DevTools(`-target wabt`のWasmビルド)でソースレベルデバッグができる

詳しくは [hike-lang/README.md](hike-lang/README.md) を参照してください。

## はじめ方

`hikec`のビルドから簡単なHello World実行までは [QuickStart.md](QuickStart.md) にまとめています。

RP2040(Raspberry Pi Pico)向けにHikeコードをビルドして実機で動かす例は [pico-blink/README.md](pico-blink/README.md) を参照してください。
