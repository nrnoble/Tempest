
#ifndef TEMPESTDISCOIO_H
#define TEMPESTDISCOIO_H

#include "TempestIO/AbstractTempestIO.h"

class TempestDiscoIO : public AbstractTempestIO {
public:
	void SetSoundChannelFrequency(int channel, int frequency);
	void SetSoundChannelVolume(int channel, int volume);
	void SetSoundChannelWaveform(int channel, int waveform);
	void Tick6KHz(void);

	void WriteVectorRAM(uint16_t address, uint8_t value);
	bool IsVectorHalt(void);
	void VectorGo(void);
	void VectorReset(void);
};

#endif
