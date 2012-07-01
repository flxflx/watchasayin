#pragma once
#include "libOTR.h"
#include "HelperFunctions.h"
#include "Logger.h"

#define PATH_EXTENSION_PRIV_KEY L"_privkey.otr"
#define PATH_EXTENSION_FINGERPRINTS L"_fingerprints.otr"

//! Describes a local OTR user.
class LocalUser
{

private:
	std::string name;
	std::wstring pathPrivKeyFile;
	std::wstring pathFingerprintsFile;
	OtrlUserState state;

public:
	/*! The public constructor.
		\param name The name of the user.
		\param state The OtrlUserState of the user.
		\param basePath The path of the directory to store/load data to/from. */
	LocalUser(const std::string& name, OtrlUserState state, const std::wstring& basePath)
	{
		std::wstring pathPK = basePath;
		pathPK += HelperFunctions::stringToWstring(name);
		pathPK += PATH_EXTENSION_PRIV_KEY;

		std::wstring pathFP = basePath;
		pathFP += HelperFunctions::stringToWstring(name); 
		pathFP += PATH_EXTENSION_FINGERPRINTS; 

		this->name = name;
		this->pathPrivKeyFile = pathPK;
		this->pathFingerprintsFile = pathFP;
		this->state = state;
	}

	//! Gets the name of the user.
	std::string getName()
	{
		return name;
	}

	//! Gets the path of the private key-file of the user.
	std::wstring getPathPrivKeyFile()
	{
		return pathPrivKeyFile;
	}

	//! Gets the path of the fngerprint store file of the user. 
	std::wstring getPathFingerprintsFile()
	{
		return pathFingerprintsFile;
	}

	//! Gets the OtrlUserState of the user.
	OtrlUserState getState()
	{
		return state;
	}
};