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

#pragma once

#include <memory>
#include <mutex>

#include <JuceHeader.h>

class DebugLog
{
	public:
		DebugLog(DebugLog const &) = delete;
		DebugLog(DebugLog &&) = delete;
		DebugLog & operator=(DebugLog const &) = delete;
		DebugLog & operator=(DebugLog &&) = delete;

		// Record a timestamped message in the log file.
		// When making multiple calls in series with reset = false, only the
		// first such message will actually be recorded; this allows e.g.
		// recording a message on first entering the processing callback in
		// the main audio thread without spamming it during rendering.
		static void log(juce::String const & name, juce::String const & msg,
				bool reset = true)
		{
			m_singleton.pLog(name, msg, reset);
		}

	private:
		DebugLog() = default;
		~DebugLog();

		static DebugLog m_singleton;

		std::unique_ptr<juce::FileLogger> m_logger;
		std::mutex m_mutex;
		bool m_init = true;
		bool m_firstLog = false;

		void pLog(juce::String const & name, juce::String const & msg,
				bool reset);
};
