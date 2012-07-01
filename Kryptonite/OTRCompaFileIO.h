#pragma once
#include "stdio.h"
#include "windows.h"

#define LIBOTR_RUNTIME_LIB L"msvcrt.dll"

//! Helper class dynamically import _wfopen_s and fclose from msvcrt.dll.
/*! Why do so? Because libOTR-2.dll uses fwrite from msvcrt.dll on our handles 
and fwrite produces an access violation in case the given file handle
was created with _wfopen_s from a different runtime lib like e.g. msvcr100.dll.*/
class OTRCompaFileIO
{
public:

	static errno_t (__cdecl* _wfopen_s) (FILE** pFile, const wchar_t *filename,  const wchar_t *mode);
	static int (__cdecl* fclose)(FILE *stream);

	static bool init()
	{
		HMODULE hRuntimeLib = LoadLibrary(LIBOTR_RUNTIME_LIB);
		if (hRuntimeLib == NULL)
		{
			return false;
		}

		if ((OTRCompaFileIO::_wfopen_s = (errno_t (__cdecl*) (FILE**, const wchar_t *,  const wchar_t *)) GetProcAddress(hRuntimeLib, "_wfopen_s")) == NULL)
		{
			FreeLibrary(hRuntimeLib);
			return false;
		}

		if ((OTRCompaFileIO::fclose = (int (__cdecl*) (FILE*)) GetProcAddress(hRuntimeLib, "fclose")) == NULL)
		{
			FreeLibrary(hRuntimeLib);
			return false;
		}
		FreeLibrary(hRuntimeLib);

		return true;
	}
};