// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// This file is part of the GameEngine developed at the
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************
//


// ****************************************************************************************
//
// GameEngine Core Library of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2012 Christoph Nuscheler
//
// ****************************************************************************************

#include "GameState.h"

#ifndef PLATFORM_WIN
#include <arpa/inet.h>
#endif

// 64-bit endianness conversion code taken from http://www.viva64.com/en/k/0018/
#define ENDIANESS_INIT 0
#define ENDIANESS_SML 1
#define ENDIANESS_BIG 2

inline uint64_t _htonll(uint64_t src) {
	static int typ = ENDIANESS_INIT;
	unsigned char c;
	union {
		uint64_t ull;
		unsigned char c[8];
	} x;
	if (typ == ENDIANESS_INIT) {
		x.ull = 0x01;
		typ = (x.c[7] == 0x01ULL) ? ENDIANESS_BIG : ENDIANESS_SML;
	}
	if (typ == ENDIANESS_BIG)
		return src;
	x.ull = src;
	c = x.c[0]; x.c[0] = x.c[7]; x.c[7] = c;
	c = x.c[1]; x.c[1] = x.c[6]; x.c[6] = c;
	c = x.c[2]; x.c[2] = x.c[5]; x.c[5] = c;
	c = x.c[3]; x.c[3] = x.c[4]; x.c[4] = c;
	return x.ull;
}

inline uint64_t _ntohll(uint64_t src) {
	static int typ = ENDIANESS_INIT;
	unsigned char c;
	union {
		uint64_t ull;
		unsigned char c[8];
	} x;
	if (typ == ENDIANESS_INIT) {
		x.ull = 0x01;
		typ = (x.c[7] == 0x01ULL) ? ENDIANESS_BIG : ENDIANESS_SML;
	}
	if (typ == ENDIANESS_BIG)
		return src;
	x.ull = src;
	c = x.c[0]; x.c[0] = x.c[7]; x.c[7] = c;
	c = x.c[1]; x.c[1] = x.c[6]; x.c[6] = c;
	c = x.c[2]; x.c[2] = x.c[5]; x.c[5] = c;
	c = x.c[3]; x.c[3] = x.c[4]; x.c[4] = c;
	return x.ull;
}


GameState::GameState()
{
	m_buffer = new char[DEFAULT_INITIAL_BUFFER_SIZE];
	m_buffersize = DEFAULT_INITIAL_BUFFER_SIZE;
	m_usedsize = 0;
	m_readindex = 0;
}

GameState::GameState(unsigned int initial_buffer_size) {
	if (initial_buffer_size == 0)	// this won't work
		initial_buffer_size = 1;
	m_buffer = new char[initial_buffer_size];
	m_buffersize = initial_buffer_size;
	m_usedsize = 0;
	m_readindex = 0;
}

GameState::~GameState()
{
	if (m_buffer)
		delete[] m_buffer;
}

unsigned int GameState::getLength() {
	return m_usedsize;
}

const char* GameState::getBuffer() {
	return (const char*) m_buffer;
}

void GameState::clear() {
	m_usedsize = 0;
	m_readindex = 0;
}

void GameState::clearAndCopyBufferFrom(char* pBuffer, unsigned int bufferSize) {
	clear();
	reserveBufferSize(bufferSize);
	memcpy(m_buffer, pBuffer, bufferSize);
	m_usedsize = bufferSize;
}

inline void GameState::reserveBufferSize(int bytes) {
	while (m_usedsize + bytes > m_buffersize) {
		// buffer space is not enough, so create a new one with double the size
		char* nbuffer = new char[m_buffersize * 2];
		memcpy(nbuffer,m_buffer,m_usedsize);
		delete[] m_buffer;
		m_buffer = nbuffer;
		m_buffersize *= 2;
	}
}

void GameState::storeByte(char value) {
	reserveBufferSize(sizeof(char));
	m_buffer[m_usedsize] = value;
	m_usedsize += sizeof(char);
}

void GameState::storeBool(bool value) {
	reserveBufferSize(sizeof(bool));
	memcpy(m_buffer + m_usedsize, &value, sizeof(bool));
	m_usedsize += sizeof(bool);
}

void GameState::storeUInt16(uint16_t value) {
	reserveBufferSize(sizeof(uint16_t));
	uint16_t nvalue = htons(value);
	memcpy(m_buffer + m_usedsize, &nvalue, sizeof(uint16_t));
	m_usedsize += sizeof(uint16_t);
}

void GameState::storeUInt32(uint32_t value) {
	reserveBufferSize(sizeof(uint32_t));
	uint32_t nvalue = htonl(value);
	memcpy(m_buffer + m_usedsize, &nvalue, sizeof(uint32_t));
	m_usedsize += sizeof(uint32_t);
}

void GameState::storeUInt64(uint64_t value) {
	reserveBufferSize(sizeof(uint64_t));
	uint64_t nvalue = _htonll(value);
	memcpy(m_buffer + m_usedsize, &value, sizeof(uint64_t));
	m_usedsize += sizeof(uint64_t);
}

void GameState::storeInt16(int16_t value) {
	storeUInt16((uint16_t)value);
}

void GameState::storeInt32(int32_t value) {
	storeUInt32((uint32_t)value);
}

void GameState::storeInt64(int64_t value) {
	storeUInt64((uint64_t)value);
}

void GameState::storeString(const char* cStr) {
	reserveBufferSize(strlen(cStr) + 1);
	memcpy(m_buffer + m_usedsize, cStr, strlen(cStr) + 1);	// including NULL termination
	m_usedsize += strlen(cStr) + 1;
}

void GameState::storeFloat(float value) {
	// We assume that floats come with the same endianness as integers, since IEEE 754 doesn't specify any.
	union {
		uint32_t ul;
		float ft;
	} float_union;

	float_union.ft = value;
	storeUInt32(float_union.ul);
}

void GameState::storeDouble(double value) {
	union {
		uint64_t ull;
		double db;
	} double_union;

	double_union.db = value;
	storeUInt64(double_union.ull);
}

int GameState::readByte(char* pByte) {
	if (m_readindex + sizeof(char) > m_usedsize)
		return -1;
	*pByte = m_buffer[m_readindex];
	m_readindex += sizeof(char);
	return 0;
}

int GameState::readBool(bool* pBool) {
	if (m_readindex + sizeof(bool) > m_usedsize)
		return -1;
	memcpy(pBool, m_buffer + m_readindex, sizeof(bool));
	m_readindex += sizeof(bool);
	return 0;
}

int GameState::readUInt16(uint16_t* pUInt16) {
	if (m_readindex + sizeof(uint16_t) > m_usedsize)
		return -1;
	uint16_t rvalue;
	memcpy(&rvalue, m_buffer + m_readindex, sizeof(uint16_t));
	m_readindex += sizeof(uint16_t);
	*pUInt16 = ntohs(rvalue);
	return 0;
}

int GameState::readUInt32(uint32_t* pUInt32) {
	if (m_readindex + sizeof(uint32_t) > m_usedsize)
		return -1;
	uint32_t rvalue;
	memcpy(&rvalue, m_buffer + m_readindex, sizeof(uint32_t));
	m_readindex += sizeof(uint32_t);
	*pUInt32 = ntohl(rvalue);
	return 0;
}

int GameState::readUInt64(uint64_t* pUInt64) {
	if (m_readindex + sizeof(uint64_t) > m_usedsize)
		return -1;
	uint64_t rvalue;
	memcpy(&rvalue, m_buffer + m_readindex, sizeof(uint64_t));
	m_readindex += sizeof(uint64_t);
	*pUInt64 = _ntohll(rvalue);
	return 0;
}

int GameState::readInt16(int16_t* pInt16) {
	return readUInt16((uint16_t*)pInt16);
}

int GameState::readInt32(int32_t* pInt32) {
	return readUInt32((uint32_t*)pInt32);
}

int GameState::readInt64(int64_t* pInt64) {
	return readUInt64((uint64_t*)pInt64);
}

int GameState::readString(char* pCStr, unsigned int cStrSize) {
	unsigned int stringlength = strnlen(m_buffer + m_readindex, m_usedsize - m_readindex);

	if (stringlength == (m_usedsize - m_readindex))		// We don't want to read beyond valid data if there is no NULL termination.
		return -1;										// So if strnlen returns the whole available buffer size, there is no valid string.

	if (cStrSize < stringlength + 1)	// not enough space in output buffer
		return -1;

	strcpy(pCStr, m_buffer + m_readindex);
	m_readindex += stringlength + 1;
	return 0;
}

int GameState::readFloat(float* pFloat) {
	union {
		uint32_t ul;
		float ft;
	} float_union;

	if (readUInt32(&float_union.ul))
		return -1;		// error retrieving 32-bits value

	*pFloat = float_union.ft;
	return 0;
}

int GameState::readDouble(double* pDouble) {
	union {
		uint64_t ull;
		double db;
	} double_union;

	if (readUInt64(&double_union.ull))
		return -1;		// error retrieving 64-bits value

	*pDouble = double_union.db;
	return 0;
}

unsigned int GameState::skipBytes(unsigned int numBytes) {
	if (m_readindex + numBytes > m_usedsize) {
		numBytes = m_usedsize - m_readindex;
	}
	m_readindex += numBytes;
	return numBytes;
}
