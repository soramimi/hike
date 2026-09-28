# Hike クイックスタート

Hike コンパイラ (`hikec`) をビルドし、Hello World を実行するまでの最短手順です。

## 前提条件

- Go 1.21+
- Clang/LLVM 15+(`clang` が `PATH` に通っていること)

バージョン確認:

```bash
go version
clang --version
```

## 1. コンパイラのビルド

リポジトリのルート(`hike-lang`)で以下を実行します。

```bash
go build -o hikec ./cmd/hikec
```

カレントディレクトリに `hikec` バイナリが生成されます。

## 2. Hello World を書く

任意のディレクトリに `main.hike` を作成します。

```go
package main

func printf(format string, ...) int

func main() {
    printf("Hello, Hike!\n")
}
```

`println` のような組み込み出力関数は無いので、libc の `printf` を外部関数として宣言して使います。

## 3. 実行する

ビルドと実行を一度に行う場合:

```bash
./hikec run main.hike
```

```
Hello, Hike!
```

## 4. ネイティブ実行ファイルとしてビルドする

```bash
./hikec build -target linux main.hike -o hello
./hello
```

`-target` には `linux` / `windows` / `windows-msvc` / `darwin` / `wasm32` / `wabt` などを指定できます。

## 次のステップ

- 標準ライブラリ(`std/...`)を使う場合はプロジェクトに `hike.mod` を用意し、`replace` や `require` で依存先を指定します(詳細は [external-module.md](external-module.md))。
- `examples/` 以下に slices・json・toml・wasm・並行処理などのサンプルがあります。
- `-g` を付けてビルドすると DWARF デバッグ情報が生成され、GDB/LLDB(ネイティブ)や Chrome DevTools(`-target wabt`)でソースレベルデバッグができます。
- 言語仕様の全体像は [README.md](README.md) の "Language Tour & Syntax Reference" を参照してください。
