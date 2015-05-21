
#ifndef TEMPESTMEMORYMAP_H
#define TEMPESTMEMORYMAP_H


const uint16_t ROM_BASE = 0x9000;
const uint16_t ROM_ECHO_START = 0xE000;
const uint16_t ROM_ECHO_END = 0xFFFF;
const uint16_t ROM_ECHO_SOURCE = 0xC000;
const uint16_t MAIN_RAM_BASE = 0x0;
const uint16_t MAIN_RAM_SIZE = 0x800;
const uint16_t COLOR_RAM_BASE = 0x0800;
const uint16_t COLOR_RAM_SIZE = 0x0010;

const uint16_t VECTOR_RAM_BASE = 0x2000;
const uint16_t VECTOR_ROM_BASE = 0x3000;
const uint16_t VECTOR_RAM_SIZE = 0x1000;
const uint16_t VECTOR_ROM_SIZE = 0x1000;

const uint16_t POKEY1_BASE = 0x60C0;
const uint16_t POKEY1_END = 0x60CF;
const uint16_t POKEY2_BASE = 0x60D0;
const uint16_t POKEY2_END = 0x60DF;
const uint16_t EEPROM_WRITE_BASE = 0x6000;
const uint16_t EEPROM_WRITE_END = 0x603F;
const uint16_t MATHBOX_WRITE_BASE = 0x6080;
const uint16_t MATHBOX_WRITE_END = 0x609F;


#endif
