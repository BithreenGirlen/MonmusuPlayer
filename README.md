# MonmusuPlayer
某TDのアレのシーン再生用。

## 動作要件
- Windows 10 以降のWindows OS
- MSVC 2015-2022 (x64)
- 游明朝
## 再生方法
フォルダ選択ダイアログから次のようなフォルダを開くと再生を開始します。

<pre>
  r18_10166_2
  ├ rev10166_02.atlas.txt
  ├ rev10166_02.png
  ├ rev10166_02.skel.txt
  ├ rev10166_022.png
  ├ rev10166_023.png
  └ ...
  </pre>

## 函式庫
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)

ビルドする際は所定の箇所に補って下さい。
  <pre>
    deps
    ├ nlohmann // JSON for Modern C++
    │   └ json.hpp
    ├ SFML-2.6.1 // static libraries and headers of SFML
    │   ├ include
    │   │   └ SFML
    │   │       └ ...
    │   └ lib
    │       └ ...
    └ spine-cpp // sources and headers of spine-cpp 3.8
        ├ include
        │   └ spine
        │       └ ...
        └ src
            └ spine
                └ ...
  </pre>

  ## マウス操作
| 入力  | 機能  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタン + マウスホイール | コマ送り加速・減速 |
| 左ボタンクリック | 次場面移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中ボタン | 尺度・速度・視点初期化。 |
| 右ボタン + マウスホイール | 文章送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |
## キー操作
| 入力  | 機能  |
| --- | --- |
| Esc | 再生終了。 |
| C   | 文字色黒・白切り替え。 |
| T   | 文章表示・非表示切り替え。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |
| PageUp | 音声加速。 |
| PageDown | 音声減速。 |
| Home | 音声等倍速。 |  
## 補足説明

### 音声再生
- SFMLが`.m4a`ファイルに対応していないため、`Microsoft Media Foundation`を利用しています。  
- 再生するには音声フォルダが選択フォルダに対して次のような位置関係にある必要があります。
<pre>
  advscene
  ├ sound
  │   ├ voice
  │   │   └ ch_10166
  │   └ ...   └ r18
  │              └ 10166a1_10166_r18
  │                 ├ 10166A0_10166_017.m4a
  │                 └ ...
  └ texture
      ├ character
      │   └ r18_scenes
      └ ...   └ r18_10166_2
  </pre>
### 文章表示
- 文章を表示するには脚本フォルダが選択フォルダに対して次のような相対位置にある必要があります。  
__選択フォルダ__  
`.../advscene/resources/advscene/texture/character/r18_scenes/r18_10166_2`  
__脚本フォルダ__  
`.../advscene/scenarioexcel/character/10166`  
- 文章には通し番号を振ってあります。
- 音声なしの場合は2秒後に、音声有りの場合は再生終了を待って次の文章に移行します。

### 立ち絵表示
次のようなファイル名称であれば表示できます。
<pre>
pose_66003_0
├ pose_66003_0.atlas.txt
├ pose_66003_0.png
├ pose_66003_0.txt // JSON形式
├ pose_66003_02.png
└ ...
</pre>
