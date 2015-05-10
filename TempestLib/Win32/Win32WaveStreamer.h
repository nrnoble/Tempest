
#ifndef WIN32WAVESTREAMER_H
#define WIN32WAVESTREAMER_H

#include "Win32WaveBuffer.h"

class AbstractTempestWaveStream;

#pragma warning(push)
#pragma warning(disable : 4820)	// padding in structures

class Win32WaveStreamer
{
public:
	Win32WaveStreamer(AbstractTempestWaveStream *source);
	~Win32WaveStreamer(void);

	std::string GetErrorString(void) const;

private:
	void	CallbackThread(void);
	void  FillBuffer(Win32WaveBuffer *buffer);
	void  ProcessFinishedBuffer(Win32WaveBuffer *buffer);

private:
	static LONG __stdcall CallbackThreadEntry(LPVOID pThis) { ((Win32WaveStreamer *)pThis)->CallbackThread(); return 0; }

private:
	AbstractTempestWaveStream *source;

	HANDLE callbackThread;
	DWORD callbackThreadID;
	HWAVEOUT waveOut;
	bool terminating;
	bool errorReported;
	std::string errorString;

	Win32WaveBuffer buffer1;
	Win32WaveBuffer buffer2;
};


#pragma warning(pop)

#endif