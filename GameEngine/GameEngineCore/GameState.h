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

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include <GameEngine/config.h>
#include <GameEngine/utMath.h>

#include <stdint.h>

class DLLEXP GameState
{
	static const int DEFAULT_INITIAL_BUFFER_SIZE = 256;

public:
	GameState();
	GameState(unsigned int initial_buffer_size);
	~GameState();

	void	storeByte(char value);
	void	storeBool(bool value);
	void	storeInt16(int16_t value);
	void	storeInt32(int32_t value);
	void	storeInt64(int64_t value);
	void	storeUInt16(uint16_t value);
	void	storeUInt32(uint32_t value);
	void	storeUInt64(uint64_t value);
	void	storeString(const char* cStr);
	void	storeFloat(float value);
	void	storeDouble(double value);
	void	storeVec3f(Horde3D::Vec3f& vec);
	void	storeVec4f(Horde3D::Vec4f& vec);
	void	storeMatrix4f(Horde3D::Matrix4f& mat);

	int		readByte(char* pByte);
	int		readBool(bool* pBool);
	int		readInt16(int16_t* pInt16);
	int		readInt32(int32_t* pInt32);
	int		readInt64(int64_t* pInt64);
	int		readUInt16(uint16_t* pUInt16);
	int		readUInt32(uint32_t* pUInt32);
	int		readUInt64(uint64_t* pUInt64);
	int		readString(char* pCStr, unsigned int cStrSize);
	int		readFloat(float* pFloat);
	int		readDouble(double* pDouble);
	int		readVec3f(Horde3D::Vec3f* pVec);
	int		readVec4f(Horde3D::Vec4f* pVec);
	int		readMatrix4f(Horde3D::Matrix4f* pMat);

	unsigned int	skipBytes(unsigned int numBytes);

	unsigned int	getLength();

	const char*		getBuffer();

	void			clear();
	void			clearAndCopyBufferFrom(char* pBuffer, unsigned int bufferSize);

private:
	char*			m_buffer;
	unsigned int	m_buffersize;
	unsigned int	m_usedsize;

	unsigned int	m_readindex;

	void	reserveBufferSize(int bytes);
};

#endif