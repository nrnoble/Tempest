/* 
 * File:   Pokey.h
 * Author: Randy
 *
 * Created on April 25, 2015, 11:14 PM
 */

#ifndef POKEY_H
#define	POKEY_H

#pragma warning(push)
#pragma warning(disable : 4820)	// padding in structures


class Pokey
{
public:
	Pokey(void);
	virtual ~Pokey(void);

	uint8_t ReadByte(uint16_t address);
	void    WriteByte(uint16_t address, uint8_t value);

	virtual uint8_t GetALLPOT(void) { return 0; }

private:
	uint8_t ALLPOT;
};


#pragma warning(pop)

#endif	/* POKEY_H */

