// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl
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
// GameEngine Animations Component
// ---------------------------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#pragma once
#ifndef STAGECONTROLLER_H_
#define STAGECONTROLLER_H_

#pragma once

#include <list>

class KeyframeAnimComponent;
class KeyframeAnimationJob;

class KeyframeStageController
{
public:
	KeyframeStageController() : Owner(0), StageID(0)
	{}

	~KeyframeStageController(void)
	{}

	void update(const float timestamp);

	void addAnimationJob( KeyframeAnimationJob* const job);

	// deletes the job in front
	void popAnimationJob();

	void clear();

	KeyframeAnimComponent*		Owner;
	std::list<KeyframeAnimationJob*>	Animations;
	unsigned int			    StageID;
};
#endif
