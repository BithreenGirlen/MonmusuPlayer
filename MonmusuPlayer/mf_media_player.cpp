

#include <atlbase.h>
#include <mfapi.h>

#include "mf_media_player.h"

CMfMediaPlayerNotify::CMfMediaPlayerNotify(void* pMediaPlayer)
	:m_pPlayer(pMediaPlayer)
{

}

CMfMediaPlayerNotify::~CMfMediaPlayerNotify()
{

}

void CMfMediaPlayerNotify::OnMediaEngineEvent(DWORD Event, DWORD_PTR param1, DWORD param2)
{
	CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pPlayer);
	if (pPlayer != nullptr)
	{
		const HWND hWnd = pPlayer->GetRetHwnd();
		const UINT uMsg = pPlayer->GetRetMsg();
		if (hWnd != nullptr && uMsg != 0)
		{
			::PostMessage(hWnd, uMsg, 0, Event);
		}
	}
}


CMfMediaPlayer::CMfMediaPlayer(HWND hWnd, UINT uMsg)
	:m_hRetWnd(hWnd), m_uRetMsg(uMsg)
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	m_hrMfStart = ::MFStartup(MF_VERSION);
	if (FAILED(m_hrMfStart))return;

	CComPtr<IMFMediaEngineClassFactory> pMfFactory;

	HRESULT hr = ::CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pMfFactory));
	if (FAILED(hr))return;

	CComPtr<IMFAttributes> pMfAttributes;

	hr = ::MFCreateAttributes(&pMfAttributes, 1);
	if (FAILED(hr))return;

	hr = pMfAttributes->SetUINT64(MF_MEDIA_ENGINE_PLAYBACK_HWND, reinterpret_cast<UINT64>(m_hRetWnd));
	if (FAILED(hr))return;

	m_pmfNotify = new CMfMediaPlayerNotify(this);
	hr = pMfAttributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, reinterpret_cast<IUnknown*>(m_pmfNotify));
	if (FAILED(hr))
	{
		m_pmfNotify->Release();
		m_pmfNotify = nullptr;
		return;
	}

	CComPtr<IMFMediaEngine> pMfMediaEngine;
	hr = pMfFactory->CreateInstance(MF_MEDIA_ENGINE_REAL_TIME_MODE, pMfAttributes, &pMfMediaEngine);
	if (FAILED(hr))
	{
		m_pmfNotify->Release();
		m_pmfNotify = nullptr;
		return;
	}

	hr = pMfMediaEngine->QueryInterface(__uuidof(IMFMediaEngineEx), (void**)&m_pmfEngineEx);
	if (FAILED(hr))
	{
		m_pmfNotify->Release();
		m_pmfNotify = nullptr;
		return;
	}

	m_pmfEngineEx->SetVolume(0.5);
	m_pmfEngineEx->SetPreload(MF_MEDIA_ENGINE_PRELOAD_METADATA);
}

CMfMediaPlayer::~CMfMediaPlayer()
{
	if (m_pmfEngineEx != nullptr)
	{
		m_pmfEngineEx->Shutdown();
		m_pmfEngineEx = nullptr;
	}

	if (SUCCEEDED(m_hrMfStart))
	{
		::MFShutdown();
		m_hrMfStart = E_FAIL;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
		m_hrComInit = E_FAIL;
	}
}

bool CMfMediaPlayer::Play(const wchar_t* pwzFilePath)
{
	if (m_pmfEngineEx != nullptr)
	{
		if (pwzFilePath != nullptr)
		{
			HRESULT hr = m_pmfEngineEx->SetSource(const_cast<BSTR>(pwzFilePath));
		}
		return SUCCEEDED(m_pmfEngineEx->Play());
	}
	return false;
}
/*再生ループ設定*/
BOOL CMfMediaPlayer::SwitchLoop()
{
	if (m_pmfEngineEx != nullptr)
	{
		m_iLoop ^= TRUE;
		if (FAILED(m_pmfEngineEx->SetLoop(m_iLoop)))
		{
			m_iLoop ^= TRUE;
		}
	}
	return m_iLoop;
}
/*消音設定*/
BOOL CMfMediaPlayer::SwitchMute()
{
	if (m_pmfEngineEx != nullptr)
	{
		m_iMute ^= TRUE;
		if (FAILED(m_pmfEngineEx->SetMuted(m_iMute)))
		{
			m_iMute ^= TRUE;
		}
	}
	return m_iMute;
}
/*音量取得*/
double CMfMediaPlayer::GetCurrentVolume()
{
	if (m_pmfEngineEx != nullptr)
	{
		return m_pmfEngineEx->GetVolume();
	}
	return 100.0;
}
/*再生速度取得*/
double CMfMediaPlayer::GetCurrentRate()
{
	if (m_pmfEngineEx != nullptr)
	{
		return m_pmfEngineEx->GetPlaybackRate();
	}
	return 1.0;
}
/*音量設定*/
bool CMfMediaPlayer::SetCurrentVolume(double dbVolume)
{
	if (m_pmfEngineEx != nullptr)
	{
		return SUCCEEDED(m_pmfEngineEx->SetVolume(dbVolume));
	}
	return false;
}
/*再生速度設定*/
bool CMfMediaPlayer::SetCurrentRate(double dbRate)
{
	if (m_pmfEngineEx != nullptr)
	{
		if (dbRate != m_pmfEngineEx->GetDefaultPlaybackRate())
		{
			m_pmfEngineEx->SetPlaybackRate(dbRate);
		}
		return SUCCEEDED(m_pmfEngineEx->SetDefaultPlaybackRate(dbRate));
	}
	return false;
}
/*再生中是否*/
bool CMfMediaPlayer::IsEnded()
{
	if (m_pmfEngineEx != nullptr)
	{
		BOOL iRet = m_pmfEngineEx->HasAudio();
		iRet |= m_pmfEngineEx->HasVideo();
		if (!iRet)
		{
			return true;
		}
		else
		{
			return m_pmfEngineEx->IsEnded() == TRUE;
		}
	}

	return true;
}
/*動画縦横幅取得*/
bool CMfMediaPlayer::GetVideoSize(DWORD* dwWidth, DWORD* dwHeight)
{
	if (dwWidth != nullptr && dwHeight != nullptr)
	{
		if (m_pmfEngineEx != nullptr)
		{
			BOOL iRet = m_pmfEngineEx->HasVideo();
			if (iRet)
			{
				HRESULT hr = m_pmfEngineEx->GetNativeVideoSize(dwWidth, dwHeight);
				return SUCCEEDED(hr);
			}
		}
	}

	return false;
}
/*表示範囲指定*/
void CMfMediaPlayer::SetDisplayArea(const RECT absoluteRect)
{
	WorkOutNormalisedRect(absoluteRect, &m_normalisedRect);
}
/*表示領域変更*/
void CMfMediaPlayer::ResizeBuffer()
{
	if (m_pmfEngineEx != nullptr)
	{
		BOOL iRet = m_pmfEngineEx->HasVideo();
		if (iRet)
		{
			RECT rc;
			::GetClientRect(m_hRetWnd, &rc);
			int iClientWidth = rc.right - rc.left;
			int iClientHeight = rc.bottom - rc.top;

			MFARGB bg{ 0, 0, 0, 0 };
			m_pmfEngineEx->UpdateVideoStream(&m_normalisedRect, &rc, &bg);
		}
	}
}
/*正規化矩形計算*/
void CMfMediaPlayer::WorkOutNormalisedRect(const RECT absoluteRect, MFVideoNormalizedRect* normaisedRect)
{
	if (m_pmfEngineEx != nullptr && normaisedRect != nullptr)
	{
		BOOL iRet = m_pmfEngineEx->HasVideo();
		if (iRet)
		{
			RECT rc;
			::GetClientRect(m_hRetWnd, &rc);
			int iClientWidth = rc.right - rc.left;
			int iClientHeight = rc.bottom - rc.top;

			float x1 = static_cast<float>(absoluteRect.left) / iClientWidth;
			float y1 = static_cast<float>(absoluteRect.top) / iClientHeight;
			float x2 = static_cast<float>(absoluteRect.right) / iClientWidth;
			float y2 = static_cast<float>(absoluteRect.bottom) / iClientHeight;

			*normaisedRect = MFVideoNormalizedRect{ x1, y1, x2, y2 };
		}
	}
}
