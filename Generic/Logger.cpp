#include "Logger.h"

vector<wstring>* Logger::lines = new vector<wstring>;
std::wstring Logger::path;
CRITICAL_SECTION Logger::csLogger;

