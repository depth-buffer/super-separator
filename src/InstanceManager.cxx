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

#include "InstanceManager.h"

InstanceManager InstanceManager::m_singleton;

void InstanceManager::registerInstance(juce::Uuid const & name,
		Remote * remote)
{
	auto l = lock();
	m_instances.emplace(name, remote);
	// TODO Look up pending "take ownership of" requests, and if there is
	// another instance waiting to take ownership of this one, provide it
	// with the Remote's pointer
}

void InstanceManager::unregisterInstance(juce::Uuid const & name)
{
	auto l = lock();
	m_instances.erase(name);
	// TODO Look up whether this instance has a leader, and if so, null out its
	// Remote pointer
}
