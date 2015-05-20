
#ifndef ABSTRACTTEMPESTIO_H
#define ABSTRACTTEMPESTIO_H

class AbstractTempestIO
{
public:
	virtual void SetSoundChannelFrequency(int channel, int frequency) = 0;
	virtual void SetSoundChannelVolume(int channel, int volume) = 0;
	virtual void SetSoundChannelWaveform(int channel, int waveform) = 0;
	virtual void Tick6KHz(void) = 0;
};

#endif