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

#include <map>
#include <mutex>

#include <JuceHeader.h>

// Forward declaration of interface class used by a leader plugin to affect
// parameter changes on a follower
class Remote;

// Singleton class for managing instance linking & cross-instance parameter
// change notifications. Just makes use of simple global variables and the fact
// that the platforms we're interested in will load only one instance of a
// shared library into a given process, so should work fine in any DAW that
// doesn't sandbox each plugin instance into its own sub-process.
class InstanceManager
{
	public:
		InstanceManager(InstanceManager const &) = delete;
		InstanceManager(InstanceManager &&) = delete;
		InstanceManager & operator=(InstanceManager const &) = delete;
		InstanceManager & operator=(InstanceManager &&) = delete;

		// Get the singleton pointer
		static InstanceManager * get()
		{
			return &m_singleton;
		}

		// Grab a copy of the current list of known plugin instances
		std::map<juce::Uuid, Remote *> instances();

		// Plugin instances should register/unregister themselves upon creation
		void registerInstance(juce::Uuid const & uuid, Remote * remote);
		void unregisterInstance(juce::Uuid const & uuid);

		// I don't know for certain that some VST hosts don't have multiple
		// GUI and audio threads, so all communication between instances should
		// be done with the manager lock held (shouldn't be a performance
		// problem as it's really not intended to have its parameters tweaked
		// except when scanning for good delay values).
		// To that end, we need:
		// - a scoped instance manager lock guard (and method for creating one)
		// - the instance manager to (with lock held) null out the follower
		//   pointer in the leader when an instance with leader unregisters
		// - leaders to check (with lock held) validity of follower pointer
		//   before calling any methods on it, so we don't crash if another
		//   thread destroys a follower during manipulation
		// - the processor destructor to take the manager lock

		[[nodiscard]] std::unique_lock<std::mutex> lock();

	private:
		InstanceManager() = default;
		~InstanceManager() = default;

		static InstanceManager m_singleton;

		std::map<juce::Uuid, Remote *> m_instances;
};
