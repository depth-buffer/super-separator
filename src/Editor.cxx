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

#include "Editor.h"
#include "SuperSeparator.h"

Editor::Editor(SuperSeparator * owner) : juce::AudioProcessorEditor(owner),
	m_paramDelay(owner->getParamDelay()),
	m_paramInvert(owner->getParamInvert()),
	m_processorParamChangeBroadcaster(owner->getChangeBroadcaster()),
	m_backgroundColour(getLookAndFeel().findColour(
				juce::ResizableWindow::backgroundColourId)),
	m_invertToggle(this, "Invert main input"),
	m_delaySlider(this, juce::Slider::LinearHorizontal,
			juce::Slider::TextBoxRight)
{
#ifdef SUPSEP_LOGGING
	owner->debugLog("Constructing editor");
#endif

	// TODO Label that converts samples to milliseconds based on current sample
	// rate. Will need a ChangeBroadcaster in the processor to update this.
	// TODO Composite component for slider, slider label, and milliseconds.

	// Set up logarithmic slider
	m_delaySlider.setRange(0, m_paramDelay.getRange().getEnd(), 1);
	m_delaySlider.setNumDecimalPlacesToDisplay(0);
	m_delaySlider.setSkewFactor(0.2);

	// Apply initial parameter values to widgets
	m_delaySlider.setValue(m_paramDelay.get(), juce::dontSendNotification);
	m_invertToggle.setToggleState(m_paramInvert.getIndex(),
			juce::dontSendNotification);

	// Attach to processor's parameter change broadcaster to update GUI in
	// response to automation/DAW-native UI parameter changes
	owner->getChangeBroadcaster().addChangeListener(&m_pluginListener);

	// Lay out GUI

	setResizable(false, false);
	setSize(320, 120);

	auto rect = getLocalBounds();
	int height = rect.getHeight() / 2;
	int constexpr margin = 10;

	m_invertToggle.setBounds(rect.removeFromTop(height).reduced(margin));
	m_delaySlider.setBounds(rect.removeFromTop(height).reduced(margin));

	addAndMakeVisible(m_invertToggle);
	addAndMakeVisible(m_delaySlider);
}

Editor::~Editor()
{
	auto ss = reinterpret_cast<SuperSeparator *>(&processor);

#ifdef SUPSEP_LOGGING
	ss->debugLog("Destroying editor");
#endif

	ss->getChangeBroadcaster().removeChangeListener(&m_pluginListener);
}

void Editor::paint(juce::Graphics & g)
{
	g.setColour(m_backgroundColour);
	g.fillAll();
}

//
// Parameter controls
//

template<typename... Args>
Editor::InvertToggle::InvertToggle(Editor * editor, Args... args)
	: juce::ToggleButton(args...), m_editor(editor)
{
}

// This is not called when setToggleState is passed dontSendNotification, which
// avoids infinite loops of host/editor callbacks from the editor constructor
void Editor::InvertToggle::clicked()
{
	m_editor->m_paramInvert.beginChangeGesture();
	m_editor->m_paramInvert.setValueNotifyingHost(getToggleState());
	m_editor->m_paramInvert.endChangeGesture();
}

template<typename... Args>
Editor::DelaySlider::DelaySlider(Editor * editor, Args... args)
	: juce::Slider(args...), m_editor(editor)
{
}

// This is only called when the UI is interacted with directly, not triggered
// as a side-effect of setValue, so we won't get host/editor callback loops
void Editor::DelaySlider::startedDragging()
{
	m_editor->m_paramDelay.beginChangeGesture();
}

// This is only called when the UI is interacted with directly, not triggered
// as a side-effect of setValue, so we won't get host/editor callback loops
void Editor::DelaySlider::stoppedDragging()
{
	m_editor->m_paramDelay.endChangeGesture();
}

// This is not called when setToggleState is passed dontSendNotification, which
// avoids infinite loops of host/editor callbacks from the editor constructor
void Editor::DelaySlider::valueChanged()
{
	m_editor->m_paramDelay.setValueNotifyingHost
		(m_editor->m_paramDelay.convertTo0to1(static_cast<float>(getValue())));
}

//
// External change listeners
//

Editor::ParentPluginChangeListener::ParentPluginChangeListener(
		Editor * editor) : m_editor(editor)
{
}

void Editor::ParentPluginChangeListener::changeListenerCallback(
		juce::ChangeBroadcaster *)
{
	// TODO If follower mode changed, toggle GUI controls enabled/disabled
	m_editor->m_delaySlider.setValue(m_editor->m_paramDelay.get(),
			juce::dontSendNotification);
	m_editor->m_invertToggle.setToggleState(m_editor->m_paramInvert.getIndex(),
			juce::dontSendNotification);
}
