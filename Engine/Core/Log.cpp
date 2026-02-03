/******************************************************************************
 * File: Log.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Logger implementation
 ******************************************************************************/

#include "Log.h"
#include <iomanip>

namespace MyEngine {

std::shared_ptr<Logger> Log::s_CoreLogger;
std::shared_ptr<Logger> Log::s_ClientLogger;

} // namespace MyEngine
