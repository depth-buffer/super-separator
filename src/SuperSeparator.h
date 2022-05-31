#pragma once

class SuperSeparator : public juce::AudioProcessor
{
	public:
		SuperSeparator() : juce::AudioProcessor(
				BusesProperties().withInput("Input",
					juce::AudioChannelSet::stereo())
				.withOutput ("Output", juce::AudioChannelSet::stereo()))
		{}

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

		// TODO
		bool hasEditor() const override
		{
			return false;
		}

		//
		// Setup & processing
		//

		// TODO
		void prepareToPlay(double sampleRate,
				int maximumExpectedSamplesPerBlock) override
		{}

		// TODO
		void releaseResources() override
		{}

		// TODO
		void processBlock(juce::AudioBuffer<float> & buffer,
				juce::MidiBuffer & midiMessages) override
		{}

		//
		// GUI
		//

		// TODO
		juce::AudioProcessorEditor * createEditor() override
		{
			return nullptr;
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
		void getStateInformation(juce::MemoryBlock & destData) override
		{}

		// TODO
		void setStateInformation(void const * data, int size) override
		{}

	private:
	    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperSeparator)
};
