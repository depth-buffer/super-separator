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
#include "InstanceManager.h"
#include "Remote.h"

InstanceManager InstanceManager::m_singleton;

bool InstanceManager::registerInstance(juce::Uuid const & name,
		Remote * remote)
{
#ifdef SUPSEP_LOGGING
	DebugLog::log("im", juce::String("Registering instance ")
			+ name.toDashedString());
#endif
	auto l = lock();
	auto i = m_instances.find(name);
	if (i == m_instances.end())
	{
		m_instances.emplace(name, remote);
		m_instancesChanged.addChangeListener(remote->getInstancesListener());
		m_instancesChanged.sendChangeMessage();
		// TODO Look up pending "take ownership of" requests, and if there is
		// another instance waiting to take ownership of this one, provide it
		// with the Remote's pointer
	}
	else if (i->second != remote)
	{
		// In an ideal world this wouldn't happen, but if the user is doing
		// something like a plugin copy & paste, it's feasible that the host
		// has created a "phantom" instance and already transferred state to it
		// (i.e. getStateInformation on the old -> setStateInformation on the
		// new) before destroying the old instance.
		// If it's a cut & paste, the old one is about to go away; but if it's
		// a copy & paste, both are going to be hanging around, and the new one
		// needs a new UUID.
#ifdef SUPSEP_LOGGING
		DebugLog::log("im",
				"Warning: duplicate UUID (plugin cut/copy & paste?)");
#endif
		return false;
	}
#ifdef SUPSEP_LOGGING
	else if (i->second == remote)
	{
		// This shouldn't happen.
		DebugLog::log("im", "Error: duplicate instance");
	}
#endif
	return true;
}

void InstanceManager::unregisterInstance(juce::Uuid const & name,
		Remote * remote)
{
#ifdef SUPSEP_LOGGING
	DebugLog::log("im", juce::String("Unregistering instance ")
			+ name.toDashedString());
#endif
	auto l = lock();
	auto i = m_instances.find(name);
	if (i != m_instances.end())
	{
		m_instancesChanged.removeChangeListener(
				i->second->getInstancesListener());
		m_instances.erase(i);
		m_instancesChanged.sendChangeMessage();
		// TODO Look up whether this instance has a leader, and if so, null out
		// its Remote pointer
	}
#ifdef SUPSEP_LOGGING
	else
	{
		DebugLog::log("im", "Error: no such instance");
	}
#endif
}

std::map<juce::Uuid, Remote *> InstanceManager::instances()
{
	auto l = lock();
	return m_instances;
}
