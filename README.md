# MonmusuPlayer
某寝室鑑賞器。

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

同梱していないのでビルドする際は所定の箇所に補って下さい。
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
### 描画領域指定
pngの大きさが縦横1024のものと2048のものが混在しており、且つ、atlasの寸法指定も2048のものと4096のものが混在しているため、読み込みに於ける上書き処理を削除しています。
```cpp
void SFMLTextureLoader::load(AtlasPage &page, const String &path) {
	sf::Texture *texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer())) return;

	if (page.magFilter == TextureFilter_Linear) texture->setSmooth(true);
	if (page.uWrap == TextureWrap_Repeat && page.vWrap == TextureWrap_Repeat) texture->setRepeated(true);

	page.setRendererObject(texture);
	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	//sf::Vector2u size = texture->getSize();
	//page.width = size.x;
	//page.height = size.y;
}
```

### 描画除外指定
一部スロットを描画から除外するよう指定しています。
```cpp
bool bFound = false;
for (size_t ii = 0; ii < m_leaveOutList.size(); ++ii)
{
	if (strstr(slot.getData().getName().buffer(), m_leaveOutList.at(ii).c_str()))
	{
		bFound = true;
		break;
	}
}
if (bFound)
{
	clipper.clipEnd(slot);
	continue;
}
```
### 合成モード指定
一部スロットを乗算合成・反転合成するよう指定しています。
```cpp
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
SFMLは`.m4a`ファイルに対応していないため、また、openALへの依存を避けるべく、Microsoft Media Foundationを利用しています。
``` cpp
	std::unique_ptr<CMediaPlayer> pMediaPlayer = std::make_unique<CMediaPlayer>(m_window->getSystemHandle());
	pMediaPlayer->SetFiles(m_audio_files);
```
音声フォルダは描画素材フォルダに対して、次のような関係位置にあると想定しています。
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
`JSON`ファイルも読み込めますが、次のような構造・拡張子形式を想定しています。
<pre>
pose_66003_0
├ pose_66003_0.atlas.txt
├ pose_66003_0.png
├ pose_66003_0.txt // JSON形式
├ pose_66003_02.png
└ ...
</pre>
