# pieceif-libusb

[P/ECE](http://aquaplus.jp/piece/) の USB 接続ライブラリである pieceif.dll を libusb に移植したものです。

## 注意

* ソース上で libusb の API に置き換えたけどまだ動かない。
* 複数接続の実装が怪しい。

## 準備

```
git clone https://github.com/autch/pieceif-libusb.git
```


本家のドライバやその他の互換ドライバが入っているときは、あらかじめデバイスマネージャで削除します。デバイスを削除する際に「このデバイスのドライバーファイルも削除します」にチェックすればいいはずです。

P/ECE を USB 接続した状態で [Zadig](http://zadig.akeo.ie/) をダウンロードして実行し、 `PIECE PME-001` が選択されている状態で WinUSB をインストールします。

[libusb 本家](http://libusb.info/) から Download -> Latest Windows Binaries をダウンロードし、以下のように配置します。

```

pieceif-libusb/
  libusb/                  ←ここに展開してリネームしておく
    include/
    MS32/
    MS64/
  pieceif-libusb.sln
  README.md
  pieceif-libusb/
    pieceif-libusb.vcxproj
    ...
```

あとは VS2017 で開けばビルドができるはずです。
