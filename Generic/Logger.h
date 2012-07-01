#pragma once
#include <string>
#include <vector>
#include "OAIdl.h"
#include "windows.h"

#ifdef DEBUG
#include <fstream>
#endif

using namespace std;

//TODO: prevent overflow

#ifdef DEBUG
#define LOG_FILENAME L"ws_dbglog.txt"
#endif

//! The logger of the module.
class Logger
{

public:
	static vector<wstring>* lines;
	static std::wstring path;
	static CRITICAL_SECTION csLogger;

	//! Inits the logger.
	static void init()
	{
		InitializeCriticalSection(&csLogger);
	}

	/*! Inits the logger.
		\param pathLogDir The path to put the log file (only used in DEBUG mode)
	*/
	static void init(wstring pathLogDir)
	{
#ifdef DEBUG
		path = pathLogDir + LOG_FILENAME;
#endif
		init();
	}

	//! Logs one line.
	static void log(wstring line)
	{
		EnterCriticalSection(&csLogger);
		lines->push_back(line);
#ifdef DEBUG
		wfstream wfs(path.c_str(), std::ios_base::out | std::ios_base::app);
		if (wfs.bad())
		{
			return;
		}
		wfs << line << endl;
		wfs.close();

#endif
		LeaveCriticalSection(&csLogger);
	}

#ifdef DEBUG
	//! Logs one line.
	static void log(string line)
	{
		EnterCriticalSection(&csLogger);
		fstream fs(path.c_str(), std::ios_base::out | std::ios_base::app);
		if (fs.bad())
		{
			return;
		}
		fs << line << endl;
		fs.close();
		LeaveCriticalSection(&csLogger);
	}
#endif
	//! Resets the logger.
	static void reset()
	{
		EnterCriticalSection(&csLogger);
		lines->clear();
#ifdef DEBUG
		fstream fs(path.c_str(), std::ios_base::out);
		fs.clear();
		fs.close();
#endif
		LeaveCriticalSection(&csLogger);
	}

	//! Logs the entrance of a function (only used in DEBUG mode).
	static void logFunctionEnter(const std::string function)
	{
#ifdef DEBUG
		std::string tmp = "Entered: ";
		tmp += function;
		log(tmp);
#endif
	}

	//! Logs the exit of a function (only used in DEBUG mode).
	static void logFunctionLeave(const std::string function)
	{
#ifdef DEBUG
		std::string tmp = "Leaving: ";
		tmp += function;
		log(tmp);
#endif
	}

	//! Gets the entire log.
	static vector<wstring>* getLog()
	{
		return lines;
	}

	//! Gets the entire log as a SafeArray (for COM).
	static SAFEARRAY* toSafeArray()
	{
		EnterCriticalSection(&csLogger);
		vector<wstring>* log = Logger::getLog();
		

		SAFEARRAY * psa;
		SAFEARRAYBOUND rgsabound[1];

		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = log->size();
		psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);

		//now iterate over all lines of the log
		vector<wstring>::iterator iter;

		LONG i;
		for (iter = log->begin(), i=0; iter != log->end(); ++iter, ++i)
		{
			//TODO: make sure we're not leaking any memory here! -> Use SysFreeString()
			SafeArrayPutElement(psa, &i, SysAllocString((const OLECHAR*)(*iter).c_str()));
		}
		LeaveCriticalSection(&csLogger);

		return psa;
	}
};