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
// ****************************************************************************************
//

// ****************************************************************************************
//
// GameEngine IK Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2010 Ionut Damian
// 
// ****************************************************************************************
#include "Config.h"
#include <string.h>
#include <XMLParser/utXMLParser.h>

#include "utils.h"

Config::Config()
{
	useDefault();
}

Config::~Config()
{
}
//DEFAULT VALUES DEFINITION
void Config::useDefault()
{
	//enum ParamI
	setParamI(IK_Param::UseDofr_I, 1);	
	setParamI(IK_Param::UseIkmo_I, 0);
	setParamI(IK_Param::UseZLock_I, 1);
	setParamI(IK_Param::CCDMaxIterations_I, 100);
	setParamI(IK_Param::CCDTwistJointManipulation_I, 1);
	setParamI(IK_Param::NumAnimFrames_I, 512);

	//enum ParamF
	setParamF(IK_Param::CCDMaxDistError_F, 0.1f);
	setParamF(IK_Param::CCDRotJitterTolerance_F, 0.00001f);
	setParamF(IK_Param::H3DAVersion_F, 2);
	setParamF(IK_Param::H3DGVersion_F, 5);
	setParamF(IK_Param::lEyeDfltRotX_F, 0);
	setParamF(IK_Param::lEyeDfltRotY_F, 90);
	setParamF(IK_Param::lEyeDfltRotZ_F, 0);
	setParamF(IK_Param::rEyeDfltRotX_F, 0);
	setParamF(IK_Param::rEyeDfltRotY_F, 90);
	setParamF(IK_Param::rEyeDfltRotZ_F, 0);

	//enum ParamS
	setParamS(IK_Param::lEyeHorizAxis_S, "Y");
	setParamS(IK_Param::rEyeHorizAxis_S, "Y");
}

void Config::loadFromXml(const XMLNode& description)
{
	//enum ParamI
	setParamI(IK_Param::UseDofr_I, atoi(description.getAttribute("UseDofr", "0")));
	setParamI(IK_Param::UseIkmo_I,  atoi(description.getAttribute("UseIkmo", "0")));
	setParamI(IK_Param::UseZLock_I,  atoi(description.getAttribute("UseZLock", "1")));
	setParamI(IK_Param::CCDMaxIterations_I,  atoi(description.getAttribute("CCDMaxIterations", "100")));
	setParamI(IK_Param::CCDTwistJointManipulation_I,  atoi(description.getAttribute("CCDTwistJointManipulation", "1")));
	setParamI(IK_Param::NumAnimFrames_I,  atoi(description.getAttribute("NumAnimFrames", "512")));

	//enum ParamF
	setParamF(IK_Param::CCDMaxDistError_F,  (float) atof(description.getAttribute("CCDMaxDistError", "0.1f")));
	setParamF(IK_Param::CCDRotJitterTolerance_F, (float) atof(description.getAttribute("CCDRotJitterTolerance", "0.00001f")));
	setParamF(IK_Param::H3DAVersion_F, (float) atof(description.getAttribute("H3DAVersion", "2")));
	setParamF(IK_Param::H3DGVersion_F, (float) atof(description.getAttribute("H3DGVersion", "5")));
	setParamF(IK_Param::lEyeDfltRotX_F, (float) atof(description.getAttribute("lEyeDfltRotX_F", "0")));
	setParamF(IK_Param::lEyeDfltRotY_F, (float) atof(description.getAttribute("lEyeDfltRotY_F", "90")));
	setParamF(IK_Param::lEyeDfltRotZ_F, (float) atof(description.getAttribute("lEyeDfltRotZ_F", "0")));
	setParamF(IK_Param::rEyeDfltRotX_F, (float) atof(description.getAttribute("rEyeDfltRotX_F", "0")));
	setParamF(IK_Param::rEyeDfltRotY_F, (float) atof(description.getAttribute("rEyeDfltRotY_F", "90")));
	setParamF(IK_Param::rEyeDfltRotZ_F, (float) atof(description.getAttribute("rEyeDfltRotZ_F", "0")));
	
	setParamF(IK_Param::lEyeHorizAxis_S, (float) atof(description.getAttribute("lEyeHorizAxis_S", "Y")));
	setParamF(IK_Param::rEyeHorizAxis_S, (float) atof(description.getAttribute("rEyeHorizAxis_S", "Y")));
}

int Config::getParamI(IK_Param::List p)
{	
	int numParam = IK_Param::_EndParamI;
	int offset = 0;

	int index = p - offset;
	if(index >= numParam || index < 0) //check array bounding
		return -1;

	return m_paramI[index]; 
}

void Config::setParamI(IK_Param::List p, int value)
{	
	int numParam = IK_Param::_EndParamI;
	int offset = 0;

	int index = p;
	if(index >= IK_Param::_EndParamI || index < 0) //check array bounding
		return;

	m_paramI[index] = value;
}

float Config::getParamF(IK_Param::List p)
{	
	int numParam = IK_Param::_EndParamF - IK_Param::_EndParamI -1;
	int offset = IK_Param::_EndParamI +1;

	int index = p - offset;
	if(index >= numParam || index < 0) //check array bounding
		return -1;

	return m_paramF[index]; 
}

void Config::setParamF(IK_Param::List p, float value)
{	
	int numParam = IK_Param::_EndParamF - IK_Param::_EndParamI -1;
	int offset = IK_Param::_EndParamI +1;

	int index = p - offset;
	if(index >= numParam || index < 0) //check array bounding
		return;

	m_paramF[index] = value;
}

const char* Config::getParamS(IK_Param::List p)
{		
	int numParam = IK_Param::_EndParamS - IK_Param::_EndParamF -1;
	int offset = IK_Param::_EndParamF +1;

	int index = p - offset;
	if(index >= numParam || index < 0) //check array bounding
		return 0;

	return m_paramS[index]; 
}

void Config::setParamS(IK_Param::List p, const char* value)
{	
	int numParam = IK_Param::_EndParamS - IK_Param::_EndParamF -1;
	int offset = IK_Param::_EndParamF +1;

	int index = p - offset;
	if(index >= numParam || index < 0) //check array bounding
		return;

	utils::strcpy( value, m_paramS[index], c_MaxSParamSize );
}

