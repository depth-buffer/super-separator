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
#include "Remote.h"
#include "SuperSeparator.h"

Remote::Remote(SuperSeparator * owner)
	: m_owner(owner), m_instancesListener(this)
{
#ifdef SUPSEP_LOGGING
	m_logname = owner->getLogName() + "-remote";
#endif
}

Remote::InstancesListener::InstancesListener(Remote * remote)
	: m_remote(remote)
{
}

void Remote::InstancesListener::changeListenerCallback(
		juce::ChangeBroadcaster *)
{
#ifdef SUPSEP_LOGGING
	DebugLog::log(m_remote->m_logname, "Notified of instance list change");
#endif
}
