//#include "Stdafx.h"

#include "PlaybackWorkerDX.h"

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <cassert>
#include "dsound.h"
#include "BackgroundExecutor.h"
#include "PlaybackController.h"

const GUID GUID_NULL = {0};

namespace Gear
{
#define method PlaybackWorkerDX::


	method PlaybackWorkerDX() :
		_playing(false),
		_primaryBuffer(nullptr),
		_secondaryBuffer(nullptr),
		_directSound(nullptr),
		_fillEvent(0)
	{
		init();
	}
	/*
	static HWND _hwnd = NULL;
	BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
	{
		DWORD lpdwProcessId;
		GetWindowThreadProcessId(hwnd, &lpdwProcessId);
		if (lpdwProcessId == lParam)
		{
			g_HWND = hwnd;
			return FALSE;
		}
		return TRUE;
	}
	*/

	static const int kPartNum = 2;
	static const int kBufferSize = 44100 * sizeof(float)* 2 / 4;

	bool method init()
	{
		HRESULT result;

		// Initialize the direct sound interface pointer for the default sound device.
		result = DirectSoundCreate8(NULL, &_directSound, NULL);
		if (FAILED(result))
		{
			return false;
		}

        GUITHREADINFO info;
        memset(&info, 0, sizeof(GUITHREADINFO));
        info.cbSize = sizeof(GUITHREADINFO);
        bool success = GetGUIThreadInfo(NULL, &info);
        assert(success);
        HWND hhwnd = info.hwndActive;
        assert(hhwnd);

		// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
		result = _directSound->SetCooperativeLevel(hhwnd, DSSCL_PRIORITY);
		if (FAILED(result))
		{
			return false;
		}

		DSBUFFERDESC bufferDesc;
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		bufferDesc.dwBufferBytes = 0;
		bufferDesc.dwReserved = 0;
		bufferDesc.lpwfxFormat = NULL;
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		// Get control of the primary sound buffer on the default sound device.
		result = _directSound->CreateSoundBuffer(&bufferDesc, &_primaryBuffer, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the format of the primary sound bufffer.
		// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
		WAVEFORMATEX waveFormat;
#if !defined(WAVE_FORMAT_IEEE_FLOAT)
#define  WAVE_FORMAT_IEEE_FLOAT         0x0003
#endif
		waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		waveFormat.nSamplesPerSec = 44100;
		waveFormat.wBitsPerSample = 32;
		waveFormat.nChannels = 2;
		waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = 0;

		// Set the primary buffer to be the wave format specified.
		result = _primaryBuffer->SetFormat(&waveFormat);
		if (FAILED(result))
		{
			return false;
		}

		// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
		bufferDesc.dwBufferBytes = kBufferSize;
		bufferDesc.dwReserved = 0;
		bufferDesc.lpwfxFormat = &waveFormat; // waveformat is ame as before
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		// Create a temporary sound buffer with the specific buffer settings.
		IDirectSoundBuffer* tempBuffer;
		result = _directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
		result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&_secondaryBuffer);
		// Release the temporary buffer.
		tempBuffer->Release();
		tempBuffer = 0;
		if (FAILED(result))
		{
			return false;
		}

		_fillEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!_fillEvent) {
			return false;
		}

		Base::BackgroundExecutor::instance().addTask([=]{
			// for now, assume that this worker lives forever

			while (true) {
				WaitForSingleObject(_fillEvent, INFINITE);
				fillBuffer();
			}
		});


		return true;
	}

	method ~PlaybackWorkerDX()
	{
		if (_fillEvent) {
			CloseHandle(_fillEvent);
		}

		if (_primaryBuffer)
		{
			_primaryBuffer->Release();
			_primaryBuffer = 0;
		}

		// Release the direct sound interface pointer.
		if (_directSound)
		{
			_directSound->Release();
			_directSound = 0;
		}
	}

	void method  fillBuffer()
	{
		HRESULT result;

		DWORD playCursor;
		static DWORD writeCursor = 0;
		DWORD safeWriteCursor; // unused
		result = _secondaryBuffer->GetCurrentPosition(&playCursor, &safeWriteCursor);
		if (FAILED(result)) {
			return;
		}
		DWORD request;
		if (playCursor > writeCursor) {
			request = playCursor - writeCursor;
		}
		else {
			request = kBufferSize - writeCursor;
		}
		if (request == 0) {
			return;
		}

		void *part1;
		void *part2;
		DWORD part1Size;
		DWORD part2Size;

		static int count = 0;
		result = _secondaryBuffer->Lock(writeCursor, request, &part1, &part1Size, &part2, &part2Size, 0);
		//printf("cursors: %d/%d  %d/%d\n", writeCursor, playCursor, part1Size, part2Size); fflush(NULL);

        auto got = generator()->getUncompressedDataInto((char *)part1, part1Size, 1);
		if (got < part1Size) {
			memset((char *)part1 + got, 0, (part1Size - got));
		}
		/*static float phase = 0;

		float *floats = (float *)part1;
		for (int i = 0; i < part1Size / sizeof(float); i += 2) {
			floats[i] = sin(phase);
			floats[i + 1] = sin(phase);

			phase += 2 * 3.14 * 440 * (1.0f / 44100.0f);
		}
		*/
		writeCursor = (writeCursor + part1Size) % kBufferSize;

		result = _secondaryBuffer->Unlock(part1, part1Size, part2, 0);
	}

	void method play()
	{
		HRESULT result;

		if (!_playing) {

			fillBuffer();
            IDirectSoundNotify *notify;
			result = _secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (void **)&notify);
			if (FAILED(result)) {
				return;
			}
			DSBPOSITIONNOTIFY descr[kPartNum];
			for (int i = 0; i < kPartNum; ++i) {
				descr[i].dwOffset = i * (kBufferSize / kPartNum);
				descr[i].hEventNotify = _fillEvent;
			}

			result = notify->SetNotificationPositions(kPartNum, descr);
			notify->Release();
			if (FAILED(result)) {
				return;
			}


			_playing = true;
			result = _secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}
	}

	void method stop(bool release)
	{
		if (_playing) {
			_playing = false;
			//_soundOut->Pause();
		}
	}

	void method setVolume(float volume)
	{
	}

	IPlaybackWorker::PcmFormat method pcmFormat()
	{
		return IPlaybackWorker::PcmFormat::Float32;
	}

    vector<AudioDevice> method availableDevices() const
    {
        return vector<AudioDevice>();
    }
}
