# MonmusuPlayer

某TDのアレのシーン再生用。

## 動作要件

- Windows 10 以降のWindows OS
- MSVC 2015-2022 (x64)

## 再生方法

以下のような階層構造から成るファイル群をご用意した上で、  
フォルダ選択ダイアログから`advscene/resources/advscene/texture/character/r18_scenes/r18_XXXXX_X`フォルダを選択すると再生を開始します。
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

  ## マウス機能
| 入力  | 機能  |
| --- | --- |
| ホイール回転 | 窓の拡大・縮小。`Ctrl`を押しながらで寄り・引き。 |
| 左押し + ホイール回転 | 動作加速・減速。 |
| 左クリック | 動作移行。同期を切った場合のみ有効。 |
| 左ドラッグ | 視点移動。 |
| 中クリック | 尺度・速度・視点初期化。 |
| 右押し + ホイール回転 | 文章送り・戻し。 |
| 右押し + 左クリック | 窓移動開始。左クリックで終了。 |

## キーボード機能
| 入力  | 機能  |
| --- | --- |
| <kbd>Esc</kbd> | 再生終了。 |
| <kbd>A</kbd> | 乗算済みアルファ有効/無効切り替え。 |
| <kbd>B</kbd> | スロット指定混成法尊重/無視切り替え。 |
| <kbd>C</kbd> | 文字色黒・白切り替え。 |
| <kbd>S</kbd>   | 画像出力。 |
| <kbd>T</kbd>   | 文章表示・非表示切り替え。 |
| <kbd>Q</kbd>   | アニメーションと文章を同期 / 独立して制御。 |
| <kbd>Up</kbd> | 前のフォルダに移動。 |
| <kbd>Down</kbd> | 次のフォルダに移動。 |
| <kbd>Right</kbd> | 文章送り。 |
| <kbd>Left</kbd> | 文章戻し。 |

## 設定

実行ファイルと同階層にある`setting.txt`から次の項目を指定できます。
- 文章を描画する際の字体ファイル
- 再生関連ファイル(Spine素材・音声・台本)の拡張子

## 外部ライブラリ

- [JSON for Modern C++ v3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)
- [SFML-3.0.2](https://www.sfml-dev.org/download/sfml/3.0.2/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## 構築方法

Visual Studioが必要になります。

1. `src/deps`階層をファイルエクスプローラーで開く。
2. `階層表示欄にcmdと入力し、コマンドプロンプトを立ち上げる。
3. `start devenv .`と入力し、Visual Studioを立ち上げる。
4. CMake設定が完了するのを待つ。
5. `MonmusuPlayer.sln`を開いてビルド。
