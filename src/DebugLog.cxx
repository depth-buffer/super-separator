// Copyright 2022 Philip Allison
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <https://www.gnu.org/licenses/>.

#include "DebugLog.h"

DebugLog DebugLog::m_singleton;

void DebugLog::pLog(juce::String const & msg, bool reset)
{
	std::unique_lock<std::mutex> l(m_mutex);

	if (m_init)
	{
		m_init = false;
		m_logger.reset(juce::FileLogger::createDateStampedLogger(
				juce::String(ProjectInfo::companyName) + '/'
					+ ProjectInfo::projectName,
				juce::String(ProjectInfo::versionString).replace(".","_")
					+ '-',
				".txt",
				juce::String(ProjectInfo::projectName) + ' '
					+ ProjectInfo::versionString));
		juce::Logger::setCurrentLogger(m_logger.get());
	}

	if (m_firstLog || reset)
	{
		m_logger->logMessage(
				juce::Time::getCurrentTime().formatted("%Y%m%d %H:%M:%S ")
				+ msg);
		m_firstLog = false;
	}
	if (reset)
		m_firstLog = true;
}

DebugLog::~DebugLog()
{
	if (!m_init)
	{
		log("Destroying logger");
		juce::Logger::setCurrentLogger(nullptr);
	}
}
