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

#include <JuceHeader.h>

// Forward declare parent class to avoid header dependency loop
class SuperSeparator;

class Remote
{
	public:
		Remote(SuperSeparator * owner);

		class InstancesListener : public juce::ChangeListener
		{
			public:
				InstancesListener(Remote * remote);
				void changeListenerCallback(juce::ChangeBroadcaster *)
					override;
			private:
				Remote * m_remote;
		};

		juce::ChangeListener * getInstancesListener()
		{
			return &m_instancesListener;
		}

		bool isAvailable() const
		{
			return m_available;
		}

	private:
		SuperSeparator * m_owner;
		InstancesListener m_instancesListener;
		bool m_available = true;

#ifdef SUPSEP_LOGGING
		juce::String m_logname;
#endif
};
