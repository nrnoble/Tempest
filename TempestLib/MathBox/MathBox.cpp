
#include "stdafx.h"

#include "MathBoxException.h"
#include "MathBoxLog.h"

#include "MathBox.h"


MathBox::MathBox(void)
{
	log = new MathBoxLog();
}

MathBox::~MathBox(void)
{
	delete log, log = NULL;
}

void MathBox::LoadROM(const uint8_t *rom, int length, char slot)
{
	switch (slot)
	{
	case 'A':
		romA.resize((unsigned)length);
		memcpy(&romA[0], rom, (unsigned)length);
		break;

	case 'E':
		romE.resize((unsigned)length);
		memcpy(&romE[0], rom, (unsigned)length);
		break;

	case 'F':
		romF.resize((unsigned)length);
		memcpy(&romF[0], rom, (unsigned)length);
		break;

	case 'H':
		romH.resize((unsigned)length);
		memcpy(&romH[0], rom, (unsigned)length);
		break;

	case 'J':
		romJ.resize((unsigned)length);
		memcpy(&romJ[0], rom, (unsigned)length);
		break;

	case 'K':
		romK.resize((unsigned)length);
		memcpy(&romK[0], rom, (unsigned)length);
		break;

	case 'L':
		romL.resize((unsigned)length);
		memcpy(&romL[0], rom, (unsigned)length);
		break;

	default:
		throw MathBoxException("MathBox::LoadROM: invalid slot letter");
	}
}


uint8_t MathBox::GetStatus(void)
{
	SetError("MathBox::GetStatus not implemented");
	return 0;
}

uint8_t MathBox::Read1(void)
{
	SetError("MathBox::Read1 not implemented");
	return 0;
}

uint8_t MathBox::Read2(void)
{
	SetError("MathBox::Read2 not implemented");
	return 0;
}

void MathBox::Write(uint8_t address, uint8_t value)
{
	try
	{
		// set our inputs... the address strobe will assert BEGIN and cause the
		// first rising edge of the clock
		addressIn = address;
		dataIn = value;
		BEGIN = true;
		HandleRisingClock();

		// shortly after the clock will fall
		HandleFallingClock();
		Log();

		// our inputs will clear
		addressIn = -1;
		dataIn = -1;
		BEGIN = false;

		// then we can just handle clock pulses until the clock is disabled
		while (!STOP.Value())
		{
			HandleRisingClock();
			HandleFallingClock();
			Log();
		}
	}
	catch (MathBoxException &x)
	{
		SetError(x.what());
	}
	catch (...)
	{
		SetError("Unknown exception in MathBox::Write");
	}
}


void MathBox::HandleRisingClock(void)
{
	// calculate the new PC 
	NullableByte newPC;
	Tristate pcen = GetTristate(PCEN);
	if (pcen.IsUnknown())
		throw MathBoxException("MathBox::HandleRisingClock: PCEN is unknown");

	if (pcen.Value())
	{
		// we load the PC from whichever source is selected
		if (BEGIN.Value())
		{
			if (addressIn < 0)
				throw MathBoxException("Load PC from ROM A: addressIn not set");
			newPC = romA[(unsigned)addressIn];
		}
		else
		{
			newPC = JumpLatch;
		}
	}
	else
	{
		if (PC.IsUnknown())
			throw MathBoxException("Can't increment PC... not set");
		newPC = (uint8_t)(PC.Value() + 1);
		if (newPC.Value() == 0)
			throw MathBoxException("PC wraparound");
	}

	// calculate the new value of Q0Latch
	Tristate newQ0Latch = GetTristate(Q0);

	// let the ALUs handle the rising clock edge...
	SetALUInputs();
	aluK.SetClock(true);
	aluF.SetClock(true);
	aluJ.SetClock(true);
	aluE.SetClock(true);

	// latch all the state values that we are supposed to latch on the
	// rising clock
	PC = newPC;
	Q0Latch = newQ0Latch;
}

void MathBox::HandleFallingClock(void)
{
	// calculate the new value of STOP
	Tristate newSTOP;
	if (BEGIN.Value())
		newSTOP = false;
	else
		newSTOP = GetTristate(A12);

	// new value of our jump address latch
	NullableByte newJumpLatch = JumpLatch;
	Tristate ldab = GetTristate(LDAB);
	if (ldab.IsUnknown() || PC.IsUnknown())
		newJumpLatch = NullableByte::Unknown;
	else
		newJumpLatch = (uint8_t)((romL[PC.Value()]<<4) + romK[PC.Value()]);

	// let the ALUs handle the falling clock edge...
	SetALUInputs();

	// When the ALU clock is high is when its gates are active... now is the time to
	// set everybody's carry flags... do it iteratively a couple times just to make sure...
	// there is some feedback here
	SetALUCarryFlags();
	SetALUCarryFlags();
	SetALUCarryFlags();
	SetALUCarryFlags();
	SetALUCarryFlags();

	// dropping the clock latches the result
	aluK.SetClock(false);
	aluF.SetClock(false);
	aluJ.SetClock(false);
	aluE.SetClock(false);

	// latch all the state values that we are supposed to latch on the
	// falling clock
	STOP = newSTOP;
	JumpLatch = newJumpLatch;
}

Tristate MathBox::GetTristate(Bit bit)
{
	char buf[200];

	switch (bit)
	{
	case A10:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romJ[PC.Value()] & 2) != 0);

	case A10STAR:
		return GetTristate(A10) ^ (GetTristate(M) && Q0Latch);

	case A12:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romH[PC.Value()] & 8) != 0);

	case A18:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romF[PC.Value()] & 2) != 0);

	case C:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romE[PC.Value()] & 1) != 0);

	case J:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romE[PC.Value()] & 4) != 0);

	case LDAB:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romF[PC.Value()] & 8) != 0);

	case M:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romE[PC.Value()] & 2) != 0);

	case PCEN:
	{
		Tristate E5XORout = GetTristate(S0) ^ GetTristate(S1);
		Tristate D4NAND1out = !(E5XORout && GetTristate(S));
		Tristate D4NAND2out = !(D4NAND1out && GetTristate(J));
		return BEGIN || !D4NAND2out;
	}

	case Q0:
		return !GetTristate(A18);

	case S:
		if (PC.IsUnknown())
			return Tristate::Unknown;
		return ((romE[PC.Value()] & 8) != 0);

	case S0:
		return aluE.GetOVR();

	case S1:
		return aluE.GetF3();

	default:
		sprintf_s(buf, "MathBox::GetTristate: unsupported bit: %d", bit);
		throw MathBoxException(buf);
	}
}


void MathBox::SetALUInputs(void)
{
	// If the PC is not set that should mean that this is our first rising edge;
	// assuming that be so we just set all the ALU inputs to their unknown state.
	// This is just a diagnostic measure so that if the ALU gets asked to do something
	// that doesn't make sense in this state it can throw an exception.
	if (PC.IsUnknown())
	{
		aluK.AAddress = aluF.AAddress = aluJ.AAddress = aluE.AAddress = NullableNybble::Unknown;
		aluK.BAddress = aluF.BAddress = aluJ.BAddress = aluE.BAddress = NullableNybble::Unknown;
		aluK.I012 = aluF.I012 = aluJ.I012 = aluE.I012 = NullableByte::Unknown;
		aluK.I345 = aluF.I345 = aluJ.I345 = aluE.I345 = NullableByte::Unknown;
		aluK.I678 = aluF.I678 = aluJ.I678 = aluE.I678 = NullableByte::Unknown;
		return;
	}

	// the A & B inputs to the ALUs are all the same
	aluK.AAddress = aluF.AAddress = aluJ.AAddress = aluE.AAddress = romL[PC.Value()];
	aluK.BAddress = aluF.BAddress = aluJ.BAddress = aluE.BAddress = romK[PC.Value()];

	// so are these
	aluK.I345 = aluF.I345 = aluJ.I345 = aluE.I345 = (uint8_t)(romH[PC.Value()] & 7);
	aluK.I678 = aluF.I678 = aluJ.I678 = aluE.I678 = (uint8_t)(romF[PC.Value()] & 7);

	// I012 are a little more complicated
	int i01 = romJ[PC.Value()] & 1;
	if (GetTristate(A10STAR).Value())
		i01 += 2;
	aluK.I012 = aluF.I012 = (uint8_t)(i01 + (romJ[PC.Value()] & 4));
	aluJ.I012 = aluE.I012 = (uint8_t)(i01 + ((romJ[PC.Value()] & 8) >> 1));

	// set the data inputs accordingly
	if (dataIn < 0)
	{
		aluK.DataIn = aluF.DataIn = aluJ.DataIn = aluE.DataIn = NullableNybble::Unknown;
	}
	else
	{
		aluK.DataIn = aluJ.DataIn = (uint8_t)(dataIn & 0xF);
		aluF.DataIn = aluE.DataIn = (uint8_t)(dataIn >> 4);
	}
}

void MathBox::SetALUCarryFlags(void)
{
	aluK.RAM0 = aluE.GetQ3();
	aluK.Q0 = GetTristate(Q0);
	aluK.CarryIn = GetTristate(C);

	aluF.RAM0 = aluK.GetRAM3();
	aluF.Q0 = aluK.GetQ3();
	aluF.CarryIn = aluK.GetCarryOut();

	aluJ.RAM0 = aluF.GetRAM3();
	aluJ.Q0 = aluF.GetQ3();
	aluJ.CarryIn = aluF.GetCarryOut();

	aluE.RAM0 = aluJ.GetRAM3();
	aluE.Q0 = aluJ.GetQ3();
	aluE.CarryIn = aluJ.GetCarryOut();
}

void MathBox::SetError(const std::string &_status)
{
	// we only record the first error
	if (error.size() == 0)
		error = _status;
}


void MathBox::Log(void)
{
	// we stop logging if we reported an error
	if (error.size() != 0)
		return;

	// create a log entry
	MathBoxLogEntry entry;

	// add bits
	entry.BEGIN = BEGIN;
	entry.PCEN = GetTristate(PCEN);
	entry.J = GetTristate(J);
	entry.S = GetTristate(S);
	entry.S0 = GetTristate(S0);
	entry.S1 = GetTristate(S1);

	// bytes
	entry.PC = PC;

	// ALU info
	entry.ALUE = aluE.GetLogData();
	entry.ALUF = aluF.GetLogData();
	entry.ALUJ = aluJ.GetLogData();
	entry.ALUK = aluK.GetLogData();

	// add it to the log
	log->AddEntry(entry);
}

std::string MathBox::GetLogXML(void) const
{
	return log->GetXML(); 
}
