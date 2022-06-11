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

#include <JuceHeader.h>

// Forward declaration of plugin editor UI
class Editor;

// Main plugin instance "entry point" class & audio processing callbacks
class SuperSeparator : public juce::AudioProcessor
{
	public:
		SuperSeparator();
		~SuperSeparator() override;

		//
		// Constants
		//

		juce::String const getName() const override
		{
			return ProjectInfo::projectName;
		}

		double getTailLengthSeconds() const override
		{
			return 0;
		}

		bool acceptsMidi() const override
		{
			return false;
		}

		bool producesMidi() const override
		{
			return false;
		}

		bool hasEditor() const override
		{
			return true;
		}

		bool supportsDoublePrecisionProcessing() const override
		{
			return true;
		}

		//
		// Setup & processing
		//

		void prepareToPlay(double sampleRate,
				int maximumExpectedSamplesPerBlock) override;
		void releaseResources() override;
		void processBlock(juce::AudioBuffer<float> & buffer,
				juce::MidiBuffer &) override;
		void processBlock(juce::AudioBuffer<double> & buffer,
				juce::MidiBuffer &) override;

		//
		// GUI
		//

		juce::AudioProcessorEditor * createEditor() override;

		juce::AudioParameterInt & getParamDelay()
		{
			return *m_paramDelay;
		}

		// TODO Instead of this, expose get/set value methods which abstract
		// away the toggle between local & remote according to follower mode,
		// so the Editor doesn't need to care
		juce::AudioParameterChoice & getParamInvert()
		{
			return *m_paramInvert;
		}

		// TODO Instead of this, expose get/set value methods which abstract
		// away the toggle between local & remote according to follower mode,
		// so the Editor doesn't need to care
		juce::ChangeBroadcaster & getChangeBroadcaster()
		{
			return m_changeBroadcaster;
		}

		//
		// Program support
		//

		int getNumPrograms() override
		{
			// Even though we don't support programs, apparently some hosts
			// don't like being told there are zero programs.
			return 1;
		}

		int getCurrentProgram() override
		{
			return 0;
		}

		void setCurrentProgram(int) override
		{}

		juce::String const getProgramName(int) override
		{
			return "Default";
		}

		void changeProgramName(int, juce::String const &) override
		{}

		//
		// State loading/saving
		//

		// TODO
		void getStateInformation(juce::MemoryBlock & destData) override;
		void setStateInformation(void const * data, int size) override;

		//
		// Debugging
		//

#ifdef SUPSEP_LOGGING
		void debugLog(juce::String const & msg, bool reset = true);
#endif

	private:
	    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperSeparator)

#ifdef SUPSEP_LOGGING
		std::unique_ptr<juce::FileLogger> m_logger;
		bool m_firstLog;
#endif

		juce::Uuid m_uuid;

		juce::dsp::DelayLine<float,
			juce::dsp::DelayLineInterpolationTypes::Linear> m_floatDelayLine;
		juce::dsp::DelayLine<double,
			juce::dsp::DelayLineInterpolationTypes::Linear> m_doubleDelayLine;

		juce::AudioParameterInt * m_paramDelay;
		juce::AudioParameterChoice * m_paramInvert;

		juce::ChangeBroadcaster m_changeBroadcaster;

		template<typename SampleType, typename DelayType> void processBlock(
				juce::AudioBuffer<SampleType> & buffer, DelayType & delayLine);
};
