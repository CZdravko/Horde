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
#include "KeyframeStageController.h"

#include "config.h"
#include "KeyframeAnimationJob.h"
#include "KeyframeAnimComponent.h"

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEntity.h>

#if not defined PLATFORM_WIN
#include <float.h>
#endif

void KeyframeStageController::update(const float timestamp)
{
	if ( Animations.empty() )  return;

	Animations.front()->update(timestamp);
	while( Animations.front()->past(timestamp) )
	{
		int jobID = Animations.front()->m_jobID;
		delete Animations.front();
		Animations.pop_front();
		if ( Animations.empty() )
		{
			if (Owner)
			{
				GameEvent event(GameEvent::E_ANIM_STOPPED, &GameEventData(jobID), Owner);
				Owner->owner()->executeEvent(&event);
			}
			return;
		}
		else Animations.front()->update(timestamp);
	}
}

void KeyframeStageController::addAnimationJob( KeyframeAnimationJob* const job)
{
	if ( !job->past(GameEngine::timeStamp()) )
	{
		while( !Animations.empty() && ( Animations.front()->past(job->m_timeline.front().Timestamp) || Animations.front()->m_endless ) )
		{
			delete Animations.front();
			Animations.pop_front();
		}
		Animations.push_back(job);
	}
}

// deletes the job in front
void KeyframeStageController::popAnimationJob()
{
	if ( Animations.empty() )  return;

	Animations.front()->unPause();
	Animations.front()->update(FLT_MAX);		// brings the animation to an end

	int jobID = Animations.front()->m_jobID;

	if (Owner)
	{
		GameEvent event(GameEvent::E_ANIM_STOPPED, &GameEventData(jobID), Owner);
		Owner->owner()->executeEvent(&event);
	}

	delete Animations.front();
	Animations.pop_front();
}

void KeyframeStageController::clear()
{
	while (!Animations.empty())
	{
		delete Animations.front();
		Animations.pop_front();
	}
}
