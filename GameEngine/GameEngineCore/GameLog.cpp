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
// Copyright (C) 2007-2011 Volker Wiendl, Felix Kistler
//
// ****************************************************************************************
#include "GameLog.h"

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <algorithm>

#ifndef PLATFORM_WIN
#include <time.h>
#include <limits.h>
#include <cstdarg>
#include <stdint.h>
#include <unistd.h>

// Since there isn't any _vscprintf equivalent in glibc, we will do our own.
// This isn't expected to be a very efficient approach, though.

// returns the number of bytes needed for a vsprintf operation
int _vscprintf(const char *format, va_list argptr) {
	int buffersize = 128;
	int result = -1;
	do {
		char* buffer = new char[buffersize];
		result = vsnprintf(buffer, buffersize - 1, format, argptr);
		delete[] buffer;
		buffersize *= 2;
	} while (result < 0);

	return result;
}
#endif

using namespace std;

namespace GameLog
{
	GAMEENGINE_API int GetExecutablePath (char* buffer, unsigned int len)
	{
		char* path_end;
		/* Read the target using GetModuleFileName  */
#ifdef PLATFORM_WIN
		if (GetModuleFileName(NULL, buffer, (DWORD)len)<= 0)
#elif defined PLATFORM_LINUX
		if (readlink ("/proc/self/exe", buffer, len) <= 0)
#endif
			return -1;
		/* Find the last occurrence of a backward slash, the path separator.  */
#ifdef PLATFORM_WIN
		path_end = strrchr (buffer, '\\');
#elif defined PLATFORM_LINUX
		path_end = strrchr (buffer, '/');
#endif
		if (path_end == NULL)
			return -1;
		/* Advance to the character past the last slash.  */
		++path_end;
		/* Obtain the directory containing the program by truncating the
		path after the last slash.  */
		*path_end = '\0';
		/* The length of the path is the number of characters up through the
		last slash.  */
		return (unsigned int) (path_end - buffer);
	}

	void findandreplace( std::string & source, const std::string & find, const std::string & replace )
	{
		for (size_t j; (j = source.find( find )) != std::string::npos;)
		{
			source.replace( j, find.length(), replace );
		}
	}

	bool isHtml(const string& fileName)
	{
		bool isHtml = false;
		size_t pos = fileName.find_last_of('.');
		if (pos != string::npos)
		{
			string extension = fileName.substr(pos);
			transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			isHtml = (extension == ".htm") || (extension == ".html");
		}
		return isHtml;
	}


	/// Map of all open logfiles
	map<string, ofstream*>	logFiles;
	/// Start time when loading the application
#if defined PLATFORM_WIN
	__int32					start = GetTickCount();
#else
	struct timespec ts;
	int waste = clock_gettime(CLOCK_MONOTONIC, &ts);;;
	int32_t start = (ts.tv_sec * 1000);;;
#endif
	/// Name of the current log file
	string					defaultLog = "_GameEngineLog.htm";

	bool initLog(const string& name)
	{
		map<string, ofstream*>::iterator iter = logFiles.find(name);
		if (iter != logFiles.end() && iter->second != 0)
		{
			if (iter->second->is_open() && iter->second->good())
				return true;
			else
			{
				delete iter->second;
				iter->second = 0;
			}
		}

#if defined PLATFORM_WIN
		char path[MAX_PATH];
#else
		char path[PATH_MAX];
#endif
		int pathLen = GetExecutablePath(path, sizeof(path));
		if (pathLen <= 0)
			return false;

		string pathStr(path);

		const char lastChar = pathStr[pathStr.length()-1];
		if (lastChar != '\\' && lastChar != '/')
			pathStr += "\\";

		pathStr += name;

		ofstream* output = new ofstream(pathStr.c_str(), ios::trunc);
		if (!output->is_open() || !output->good())
			return false;

		logFiles[name] = output;

		if (isHtml(name))
		{
			*output<<
				"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
				"<html>\n"
				"<head>\n"
				"<title>";
			if (name.compare(defaultLog) != 0)
				*output << name << " - ";
			*output<<
				"GameEngine Logfile</title>\n"
				"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
				"</head>\n"
				"<body bgcolor=\"#FAF1D3\">\n"
				"<p align=\"center\"><strong><font size=\"+2\">";
			if (name.compare(defaultLog) != 0)
				*output << name << " - ";
			*output<<
				"GameEngine Logfile</font></strong></p>\n"
				"<p align=\"center\">Created with GameEngine Log 1.1</p>\n"
				"<hr>\n"
				"<p>Debuginformation follows:</p>\n"
				"<center>\n"
				"<table width=\"75%\" border=\"0\">\n";
		}
		return output->is_open() && output->good();
	}

	void close()
	{
		map<string, ofstream*>::iterator iter;
		map<string, ofstream*>::iterator end = logFiles.end();
		for (iter = logFiles.begin(); iter != end; ++iter)
		{
			ofstream* output = iter->second;
			if (output !=0 )
			{
				if (isHtml(iter->first))
				{
					*output << "</table>\n"
						<< "</center>\n"
						<< "<p><font size=\"-2\">GameLog - University of Augsburg (2011)</font></p>\n"
						<< "</body>\n"
						<< "</html>\n";
				}
				output->close();
				delete output;
			}
		}
		logFiles.clear();
	}

	void logHtml(const string& logFileName, const char* bgcolor, const char* msg)
	{
		if (!initLog(logFileName))
		{
			cerr << "Error writing to log file" << endl;
			return;
		}
		ofstream* output = logFiles[logFileName];

		std::string cleanMsg(msg);

		findandreplace(cleanMsg, ">", "&gt;");
		findandreplace(cleanMsg, "<", "&lt;");
		*output << "\t<tr>\n";

#if defined PLATFORM_WIN
		__int32 currentTime = GetTickCount();
#else
		clock_gettime(CLOCK_MONOTONIC, &ts);
		int32_t currentTime = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif

		*output	<< "\t\t<td width=\"23%\" bgcolor=\"" << bgcolor << "\"><font color=\"#FFFFFF\"><strong>" << (currentTime - start) * 0.001f << "s</strong></font></td>\n"
			<< "\t\t<td width=\"77%\" bgcolor=\"" << bgcolor << "\"><font color=\"#FFFFFF\"><strong>" << cleanMsg << " ";
		*output << "</strong></font></td>\n"
			<< "\t</tr>\n";
		(*output).flush();
	}

	void logPlain(const string& logFileName, const char* msg)
	{
		if (!initLog(logFileName))
		{
			cerr << "Error writing to log file" << endl;
			return;
		}
		ofstream* output = logFiles[logFileName];

		(*output)	<< msg << "\n";

		output->flush();
	}

	void log(const string& logFileName, const char* bgcolor, const char* msg)
	{
		if (isHtml(logFileName))
			logHtml(logFileName, bgcolor, msg);
		else
			logPlain(logFileName, msg);
	}

	void log(const char* bgcolor, const char* msg)
	{
		logHtml(defaultLog, bgcolor, msg);
	}

	GAMEENGINE_API void logToCustomFile(const char* logFileName, const char* message, ...)
	{
		if (logFileName)
		{
			va_list arglist;
			va_start( arglist, message );
			int len = _vscprintf( message, arglist )+ 1;
			char* buffer = new char[len * sizeof(char)];
			// reset argument list, was possibly worked through by _vscprintf
			va_start( arglist, message );
			#pragma warning( push )
			#pragma warning( disable:4996 )
			vsprintf( buffer, message, arglist );
			#pragma warning( pop )

			log(logFileName, "#009900", buffer);
			delete[] buffer;
		}
	}

	GAMEENGINE_API void logMessage(const char* message, ...)
	{
		va_list arglist;
		va_start( arglist, message );
		int len = _vscprintf( message, arglist )+ 1;
		char* buffer = new char[len * sizeof(char)];
		// reset argument list, was possibly worked through by _vscprintf
		va_start( arglist, message );
		#pragma warning( push )
		#pragma warning( disable:4996 )
		vsprintf( buffer, message, arglist );
		#pragma warning( pop )

		log("#009900", buffer);
		delete[] buffer;
	}

	GAMEENGINE_API void warnMessage(const char* message, ...)
	{
		va_list arglist;
		va_start( arglist, message );
		int len = _vscprintf( message, arglist )+ 1;
		char* buffer = new char[len * sizeof(char)];
		// reset argument list, was possibly worked through by _vscprintf
		va_start( arglist, message );
		#pragma warning( push )
		#pragma warning( disable:4996 )
		vsprintf( buffer, message, arglist );
		#pragma warning( pop )

		log("#FFCC00", buffer);
		delete[] buffer;
	}

	GAMEENGINE_API void errorMessage(const char* message, ...)
	{
		va_list arglist;
		va_start( arglist, message );
		int len = _vscprintf( message, arglist )+ 1;
		char* buffer = new char[len * sizeof(char)];
		// reset argument list, was possibly worked through by _vscprintf
		va_start( arglist, message );
		#pragma warning( push )
		#pragma warning( disable:4996 )
		vsprintf( buffer, message, arglist );
		#pragma warning( pop )

		log("#FF0000", buffer);
		delete[] buffer;
	}
}
