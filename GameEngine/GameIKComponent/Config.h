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
#ifndef IK_CONFIG_H_
#define IK_CONFIG_H_

#include "GameEngine_IK.h"

//////////////////////////////////////////////
//Definitions
//#define IK_DEBUG //requires gazeaux resource (models/gazeaux/gazeaux.scene.xml)
//////////////////////////////////////////////
const size_t c_MaxSParamSize = 64;
//////////////////////////////////////////////

struct XMLNode;

/**
 * Contains configurations for all IK processes
 *
 * @author Ionut Damian
 */
class Config
{
private:	
	int m_paramI[ IK_Param::_EndParamI ];
	float m_paramF[ IK_Param::_EndParamF - IK_Param::_EndParamI -1 ];
	char m_paramS[ IK_Param::_EndParamS - IK_Param::_EndParamF -1 ][c_MaxSParamSize];

public:

	Config();
	~Config();
	/**
	 * Initializes all entries in IK_Param with default values
	 * THE DEFAULT VALUES ARE STORED LOCALLY IN THIS FUNCITON
	 */
	void useDefault();

	/**
	 * \brief Loads the config from an xml node
	 * 
	 * @param description xmlnode with ik config
	 */ 
	void			loadFromXml(const XMLNode& description);

	int				getParamI(IK_Param::List p);
	float			getParamF(IK_Param::List p);
	const char*		getParamS(IK_Param::List p);

	void			setParamI(IK_Param::List p, int value);
	void			setParamF(IK_Param::List p, float value);	
	void			setParamS(IK_Param::List p, const char* value);
};

#endif