# MonmusuPlayer
某TDのアレのシーン再生用。

## 動作要件
- Windows 10 以降のWindows OS
- MSVC 2015-2022 (x64)
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

## Libraries
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

ビルドする際は所定の箇所に補って下さい。
  <pre>
    deps
    ├ SFML-2.6.1 // 上記リンクから取得
    │   ├ include
    │   │   └ SFML
    │   │       └ ...
    │   └ lib
    │       └ ...
    ├ spine-cpp-3.8 // 上記リンクから取得
    │   ├ include
    │   │   └ spine
    │   │       └ ...
    │   └ src
    │       └ spine
    │           └ ...
    └ spine-sfml-3.8 // 同梱済み
        ├ spine-sfml.cpp
        └ spine-sfml.h
  </pre>

  ## マウス操作
| 入力  | 機能  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタン + マウスホイール | コマ送り加速・減速 |
| 左ボタンクリック | 次場面移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中央ボタン | 拡縮・速度・視点初期化。 |
| 右ボタン + マウスホイール | 音声送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |
## キー操作
| 入力  | 機能  |
| --- | --- |
| Esc | 再生終了。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |
| PageUp | 音声加速。 |
| PageDown | 音声減速。 |
| Home | 音声等倍速。 |  
## 補足説明
### 読み込み処理
pngの大きさが縦横1024のものと2048のものが混在しており、また、atlasの寸法指定も2048のものと4096のものが混在しているため、`spine-sfml.cpp`に於ける読み込み処理の一部を削除しています。
```cpp
void SFMLTextureLoader::load(AtlasPage &page, const String &path) {
	/*中略*/
	page.setRendererObject(texture);
	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	//sf::Vector2u size = texture->getSize();
	//page.width = size.x;
	//page.height = size.y;
}
```

### 描画
`sfml_spine_player.cpp`にて描画から除外するスロット、乗算合成・反転合成するスロットを指定しています。  
但し、乗算合成に関してはアルファ値をみて最終的に適用するか判断します。
```cpp
drawable->SetLeaveOutList(leaveOutList);
drawable->SetBlendMultiplyList(blendMultiplyList);

auto& slots = m_skeletonData.at(i).get()->getSlots();
for (size_t ii = 0; ii < slots.size(); ++ii)
{
	std::string strName = slots[ii]->getName().buffer();
	for (const std::string& str : blendScreenList)
	{
		if (strstr(strName.c_str(), str.c_str()))
		{
			slots[ii]->setBlendMode(spine::BlendMode::BlendMode_Screen);
		}
	}
}
```
### 音声再生
SFMLは`.m4a`ファイルに対応していないため、Microsoft Media Foundationを利用しています。OpenALの使用を避けるという狙いもあります。
``` cpp
std::unique_ptr<CMediaPlayer> pMediaPlayer = std::make_unique<CMediaPlayer>(m_window->getSystemHandle());
pMediaPlayer->SetFiles(m_audio_files);
```
再生するには音声フォルダが選択フォルダに対して次のような位置関係にある必要があります。
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
