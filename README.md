# MonmusuPlayer
某TDのアレのシーン再生用。

## 動作要件
- Windows 10 以降のWindows OS
- MSVC 2015-2022 (x64)

## 再生方法

階層構造を保持するよう注意しつつ、以下のようなファイル群をご用意下さい。  
フォルダ選択ダイアログから`advscene/resources/advscene/texture/character/r18_XXXXX_X`フォルダを選択すると再生を開始します。
<pre>
advscene
├ resources
│  └ advscene
│    ├ ...
│    ├ sound
│    │  ├ ...
│    │  ├ voice // 音声フォルダ
│    │  │  ├ ...
│    │  │  ├ ch_10166
│    │  │  │  ├ ...
│    │  │  │  └ r18
│    │  │  │    ├ ...
│    │  │  │    ├ 10166a1_10166_r18
│    │  │  │    │  ├ 10166A0_10166_017.m4a
│    │  │  │    │  └ ...
│    │  │  │    ├ ...
│    │  │  │    └ 10166b1_10166_r18
│    │  │  └ ...
│    └ texture
│      ├ ...
│      ├ character // Spine素材フォルダ
│      │  ├ chr_poses
│      │  └ r18_scenes
│      │    ├ ...
│      │    ├ r18_10166_1
│      │    ├ r18_10166_2 // この階層のフォルダを選択
│      │    │  ├ rev10166_02.atlas.txt
│      │    │  ├ rev10166_02.png
│      │    │  ├ rev10166_02.skel.txt
│      │    │  ├ rev10166_022.png
│      │    │  └ rev10166_023.png
│      │    └ ...
│      └ ...
├ ...
└ scenarioexcel
  ├ ...
  ├ character // 台本フォルダ
  │  ├ ...
  │  ├ 10166
  │  │  ├ ...
  │  │  ├ 10166A1_R18.book.json
  │  │  ├ ...
  │  │  └ 10166B1_R18.book.json
  │  └ ...
  └ ...
</pre>

  ## マウス操作
| 入力  | 機能  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタン + マウスホイール | 動作加速・減速 |
| 左ボタンクリック | 動作移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中ボタン | 尺度・速度・視点初期化。 |
| 右ボタン + マウスホイール | 文章送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |

## キー操作
| 入力  | 機能  |
| --- | --- |
| Esc | 再生終了。 |
| A   |乗算済みアルファ有効/無効切り替え。 |
| B   | スロット指定混成法尊重/無視切り替え。 |
| C   | 文字色黒・白切り替え。 |
| T   | 文章表示・非表示切り替え。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |
| PageUp | 音声加速。 |
| PageDown | 音声減速。 |
| Home | 音声等倍速。 |  

## 設定

実行ファイルと同階層にある`setting.txt`から次の項目を指定できます。
- 文章を描画する際の字体ファイル
- 再生関連ファイル(Spine素材・音声・台本)の拡張子

## 外部ライブラリ
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
- [SFML-2.6.2](https://www.sfml-dev.org/download/sfml/2.6.2/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## 構築方法

1. `MonmusuPlayer/deps/CMakeLists.txt`を実行して外部ライブラリを取得。
2. Visual Studioから`MonmusuPlayer.sln`を開く。
3. メニュー欄`ビルド`から`ソリューションのビルド`を選択。

`MonmusuPlayer/deps`以下は次のような構成になります。
<pre>
deps
├ nlohmann // JSON for Modern C++
│  └ json.hpp
├ SFML-2.6.2 // VC17向けSFML
│  ├ include
│  │  └ SFML
│  │    └ ...
│  └ lib
│       └ ...
└ spine-cpp // Spine 3.8向けのC++汎用ランタイム
   ├ include
   │  └ spine
   │    └ ...
   └ src
    └ spine
        └ ...
</pre>
