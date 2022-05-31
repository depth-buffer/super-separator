#include "SuperSeparator.h"

namespace
{
	class DelayParam : public juce::AudioParameterInt
	{
		public:
			DelayParam(SuperSeparator * proc, juce::String const & parameterID,
					juce::String const & parameterName, int minValue,
					int maxValue, int defaultValue,
					juce::String const & parameterLabel)
				: juce::AudioParameterInt(parameterID, parameterName, minValue,
						maxValue, defaultValue, parameterLabel), m_proc(proc)
			{}

			bool isDiscrete() const override
			{
				return true;
			}

		private:
			SuperSeparator * m_proc;

			void valueChanged(int newValue) override
			{
#ifdef SUPSEP_LOGGING
				juce::String d("valueChanged: ");
				d += newValue;
				m_proc->debugLog(d);
#endif
			}
	};
}

// JUCE framework entry point - construct new instance of the plugin
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter()
{
    return new SuperSeparator();
}

SuperSeparator::SuperSeparator() : juce::AudioProcessor(
		BusesProperties()
		.withInput("Input", juce::AudioChannelSet::stereo())
		.withInput("Sidechain", juce::AudioChannelSet::stereo())
		.withOutput("Output", juce::AudioChannelSet::stereo())),
#ifdef SUPSEP_LOGGING
	m_logger(juce::FileLogger::createDateStampedLogger(
				juce::String(ProjectInfo::companyName) + '/'
					+ ProjectInfo::projectName,
				juce::String(ProjectInfo::versionString).replace(".","_")
					+ '-',
				".txt",
				juce::String("Hello, world! ") + ProjectInfo::versionString)),
	m_firstLog(true),
#endif
	m_sampleRate(44100), m_mainInvert(1), m_sideInvert(-1),
	m_floatDelayLine(5760), m_doubleDelayLine(5760),
	// 5760 = 15 * 384, i.e. enough samples to go up to 15ms delay at
	// 384kHz. Should be enough for anyone, right...?
	m_paramDelay(new DelayParam(this, "delay", "Delay", 0, 5760, 1,
				"samples")),
	m_paramInvert(new juce::AudioParameterChoice("invert", "Invert",
				{"Primary", "Secondary"}, 0))
{
	// TODO: Parameters
	// Per-channel delay gain
	// Dry/wet

	// TODO If multi-instance linking does indeed work out:
	// Delay times linked
	// Naming on invert parameter change from "A/B" to "Yes/No" & linked
	// Dry/wet linked
	addParameter(m_paramDelay);
	addParameter(m_paramInvert);
}

//
// Utility functions
//

#ifdef SUPSEP_LOGGING
void SuperSeparator::debugLog(juce::String const & msg, bool reset)
{
	if (m_firstLog || reset)
	{
		m_logger->logMessage(
				juce::Time::getCurrentTime().formatted("%Y%m%d %H:%M:%S ")
				+ msg);
		m_firstLog = false;
	}
	if (reset)
		m_firstLog = true;
}
#endif

juce::String SuperSeparator::delayString(int value, int maxlen) const
{
	double ms = static_cast<double>(value) / (m_sampleRate / 1000.0);
	juce::String str(ms);
	str += " milliseconds";
	return str.substring(0, maxlen);
}

//
// Setup & processing
//

void SuperSeparator::prepareToPlay(double sampleRate,
		int maximumExpectedSamplesPerBlock)
{
#ifdef SUPSEP_LOGGING
	debugLog(juce::String("prepareToPlay: ") + juce::String(sampleRate) + ' '
			+ juce::String(maximumExpectedSamplesPerBlock));
#endif

	m_sampleRate = sampleRate;

	juce::dsp::ProcessSpec spec {sampleRate,
		static_cast<uint32_t>(maximumExpectedSamplesPerBlock), 4};

	if (getProcessingPrecision() == singlePrecision)
	{
		m_floatDelayLine.setDelay(static_cast<float>(m_paramDelay->get()));
		m_floatDelayLine.prepare(spec);
	}
	else
	{
		m_doubleDelayLine.setDelay(static_cast<float>(m_paramDelay->get()));
		m_doubleDelayLine.prepare(spec);
	}

	if (m_paramInvert->getIndex() == 0)
	{
		m_mainInvert = 1;
		m_sideInvert = -1;
	}
	if (m_paramInvert->getIndex() == 1)
	{
		m_mainInvert = -1;
		m_sideInvert = 1;
	}
}

void SuperSeparator::releaseResources()
{
#ifdef SUPSEP_LOGGING
	debugLog("releaseResources");
#endif
}

// As nothing we're doing is specific to float or double type, support both
// with a single private template method, instantiated inside both the float &
// double public processing methods.
template<typename SampleType, typename DelayType>
void SuperSeparator::processBlock(juce::AudioBuffer<SampleType> & buffer,
		DelayType & delayLine)
{
	// TODO Make sure we code defensively - number of samples in the buffer
	// might be bigger than maximumExpectedSamplesPerBlock.
	// Check whether DelayLine handles this natively or just barfs - if the
	// latter, we will need to chop up blocks here before passing them in.
		m_floatDelayLine.setDelay(static_cast<float>(m_paramDelay->get()));
		m_floatDelayLine.setDelay(static_cast<float>(m_paramDelay->get()));

	if (m_paramInvert->getIndex() == 0)
	{
		m_mainInvert = 1;
		m_sideInvert = -1;
	}
	if (m_paramInvert->getIndex() == 1)
	{
		m_mainInvert = -1;
		m_sideInvert = 1;
	}

	auto main = getBusBuffer(buffer, true, 0);
	auto side = getBusBuffer(buffer, true, 1);

	SampleType const ** pmain = main.getArrayOfReadPointers();
	SampleType const ** pside = side.getArrayOfReadPointers();
	SampleType ** dst = main.getArrayOfWritePointers();

	for (int i = 0; i < buffer.getNumSamples(); ++i)
	{
		for (int j = 0; j < main.getNumChannels(); ++j)
		{
			delayLine.pushSample(j, pmain[j][i]
					* static_cast<SampleType>(m_mainInvert));
			dst[j][i] = delayLine.popSample(j) + pmain[j][i];
		}
	}

	for (int i = 0; i < buffer.getNumSamples(); ++i)
	{
		for (int j = 0; j < side.getNumChannels(); ++j)
		{
			delayLine.pushSample(j + 2, pside[j][i]
					* static_cast<SampleType>(m_sideInvert));
			dst[j][i] += delayLine.popSample(j + 2) + pside[j][i];
		}
	}
}

void SuperSeparator::processBlock(juce::AudioBuffer<float> & buffer,
		juce::MidiBuffer &)
{
#ifdef SUPSEP_LOGGING
	juce::String d("processBlock<float>: ");
	d += buffer.getNumSamples();
	d += " samples";
	debugLog(d, false);
#endif
	processBlock(buffer, m_floatDelayLine);
}

void SuperSeparator::processBlock(juce::AudioBuffer<double> & buffer,
		juce::MidiBuffer &)
{
#ifdef SUPSEP_LOGGING
	juce::String d("processBlock<double>: ");
	d += buffer.getNumSamples();
	d += " samples";
	debugLog(d, false);
#endif
	processBlock(buffer, m_doubleDelayLine);
}
