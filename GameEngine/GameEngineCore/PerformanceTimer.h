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
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#ifndef PERFORMANCETIMER_H_
#define PERFORMANCETIMER_H_

#if defined ( WIN32 ) || defined( _WINDOWS )
#define PLATFORM_WIN
#else
#include <inttypes.h>
#endif

/** \addtogroup GameEngineCore
 * @{
 */

template <typename T>
class PerformanceTimer
{
public:
	PerformanceTimer();

	// resets the timer
	void resetTimer();
	// returns the elapsed time in seconds
	T elapsedTime();

private:
	T		m_timeScale;
	bool	m_highPerformance;
#if defined PLATFORM_WIN
	__int64 m_lastTime;
#else
	int64_t m_lastTime;
#endif
};

/*! @}*/
#endif
