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

class SuperSeparator;

// Class for the plugin's editor GUI
class Editor : public juce::AudioProcessorEditor
{
	public:
		Editor(SuperSeparator * owner);
		~Editor() override;

		void paint(juce::Graphics & g) override;

	private:
	    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)

#ifdef SUPSEP_LOGGING
		juce::String m_logname;
#endif

		// Move inherited public data members to private; we don't want
		// stuff messing with them unnecessarily
		using juce::AudioProcessorEditor::processor;
		using juce::AudioProcessorEditor::resizableCorner;

		// References to audio processor's parameters
		juce::AudioParameterInt & m_paramDelay;
		juce::AudioParameterChoice & m_paramInvert;

		// And the change broadcaster on which it will broadcast when there's
		// been a parameter change via automation/DAW-native UI
		juce::ChangeBroadcaster & m_processorParamChangeBroadcaster;

		juce::Colour m_backgroundColour;

		//
		// Parameter controls
		//

		class InvertToggle : public juce::ToggleButton
		{
			public:
				template<typename... Args>
				InvertToggle(Editor * editor, Args... args);

			private:
				Editor * m_editor;
				void clicked() override;
		};

		class DelaySlider : public juce::Slider
		{
			public:
				template<typename... Args>
				DelaySlider(Editor * editor, Args... args);

				void startedDragging() override;
				void stoppedDragging() override;
				void valueChanged() override;

			private:
				Editor * m_editor;
		};

		InvertToggle m_invertToggle;
		DelaySlider m_delaySlider;

		//
		// External change listeners
		//

		// Listener for parameter changes coming from our parent
		// AudioProcessor instance, e.g. from parameter automation during
		// playback or host-native GUI controls
		class ParentPluginChangeListener : public juce::ChangeListener
		{
			public:
				ParentPluginChangeListener(Editor * editor);
				void changeListenerCallback(juce::ChangeBroadcaster *)
					override;

			private:
				Editor * m_editor;
		};

		ParentPluginChangeListener m_pluginListener{this};

		// TODO ChangeListeners for:
		// - Remote's ChangeBroadcaster (parameter changes when in follower
		//   mode)
		// - InstanceManager's ChangeBroadcaster (changes to/from follower
		//   mode; plugin instances registered/unregistered)
		// - Or should the concept of a Remote be mostly abstracted away
		//   within the SuperSeparator class itself, and the Editor always
		//   listens to the parent plugin's change broadcaster?
		//   - Probably cleaner
		//   - Have the plugin expose a "follower mode" toggle, and use that to
		//     decide in the Editor when to enable/disable the GUI controls
		//   - Have the SuperSeparator expose methods for getting/setting
		//     parameter values, instead of just direct references to the JUCE
		//     parameters, and abstract away the toggle between local & remote
		//     values
};
