# pieceif-libusb

[P/ECE](http://aquaplus.jp/piece/) の USB 接続ライブラリである pieceif.dll を libusb に移植したものです。

ドライバ署名検証の無効化というリスクを冒さずに P/ECE を現代の Windows で使うことができるようになります。

## 注意

* ~~ソース上で libusb の API に置き換えたけどまだ動かない。~~ → 直した。動く。
* ~~複数接続の実装が怪しい。~~ → 動く？

## 準備

本家のドライバやその他の互換ドライバが入っているときは、あらかじめデバイスマネージャで削除します。デバイスを削除する際に「このデバイスのドライバーファイルも削除します」にチェックすればいいはずです。

P/ECE を USB 接続した状態で [Zadig](http://zadig.akeo.ie/) をダウンロードして実行し、 `PIECE PME-001` が選択されている状態で WinUSB をインストールします。

## バイナリ配布を使う

[Visual Studio 2017 の Microsoft Visual C++ 再頒布可能パッケージ (x86)](https://go.microsoft.com/fwlink/?LinkId=746572) が入っていなければ入れておきます。

P/ECE 開発環境の `bin/pieceif.dll` を、配布 zip の中にあるファイル一式で（バックアップを取ってから）上書きしてください。

バイナリ配布には [libusb 1.0](http://libusb.info/) を含みます。LGPLv2.1 です。

## ビルドする

```
git clone https://github.com/autch/pieceif-libusb.git
```

[libusb 本家](http://libusb.info/) から Download -> Latest Windows Binaries をダウンロードし、以下のように配置します。

```
pieceif-libusb/
  libusb/                  ←ここに展開してリネームしておく
    include/
    MS32/
      dll/
        libusb-1.0.lib
        libusb-1.0.dll
    MS64/
  pieceif-libusb.sln
  README.md
  pieceif-libusb/
    pieceif-libusb.vcxproj
    ...
```

あとは VS2017 で開けばビルドができるはずです。

できあがった pieceif.dll と libusb-1.0.dll を P/ECE 開発環境の `piece\bin` ディレクトリに上書きコピーすれば、`isd` や `WinIsd` が使えるようになるはずです。
