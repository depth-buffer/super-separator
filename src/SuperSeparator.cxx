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

#ifdef SUPSEP_LOGGING
#include <sstream>
#endif

#include "DebugLog.h"
#include "Editor.h"
#include "InstanceManager.h"
#include "SuperSeparator.h"

namespace
{
	// Base class for parameters that send notifications to their owning
	// SuperSeparator's embedded change broadcaster when altered
	template<class ParamType>
	class ChangeBroadcastedParam : public ParamType
	{
		public:
			template<typename... Args>
			ChangeBroadcastedParam(SuperSeparator * proc, Args... args)
				: ParamType(args...), m_proc(proc)
			{}

			bool isDiscrete() const override
			{
				return true;
			}

		protected:
			SuperSeparator * m_proc;

			void valueChanged(int newValue) override
			{
#ifdef SUPSEP_LOGGING
				juce::String d{ParamType::getParameterID()};
				d += " param valueChanged: ";
				d += newValue;
				DebugLog::log(m_proc->getLogName(), d);
#endif
				m_proc->getChangeBroadcaster().sendChangeMessage();
			}
	};
}

//
// Constructor & destructor
//

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
	m_floatDelayLine(5760), m_doubleDelayLine(5760),
	// 5760 = 15 * 384, i.e. enough samples to go up to 15ms delay at
	// 384kHz. Should be enough for anyone, right...?
	m_paramDelay(new ChangeBroadcastedParam<juce::AudioParameterInt>
			(this, "delay", "Delay", 0, 5760, 1, "samples")),
	m_paramInvert(new ChangeBroadcastedParam<juce::AudioParameterChoice>
			(this, "invert", "Invert",
			 juce::StringArray{"Secondary", "Primary"}, 0))
{
	// TODO: Future parameters?
	// Per-channel delay gain
	// Dry/wet (instance-linked)

	// TODO If multi-instance linking does indeed work out:
	// Delay times linked
	// Naming on invert parameter change from "A/B" to "Yes/No" & linked
	// Dry/wet linked
	addParameter(m_paramDelay);
	addParameter(m_paramInvert);

#ifdef SUPSEP_LOGGING
	m_logname = juce::String::toHexString(m_uuid.hash());
	DebugLog::log(m_logname,
			juce::String("Instance UUID: ") + m_uuid.toDashedString());
	std::ostringstream oss;
	oss << "Instance manager: " << std::ios::hex << InstanceManager::get();
	DebugLog::log(m_logname, oss.str());
#endif

	// TODO Pass Remote's pointer
	InstanceManager::get()->registerInstance(m_uuid, nullptr);
}

SuperSeparator::~SuperSeparator()
{
#ifdef SUPSEP_LOGGING
	DebugLog::log(m_logname, juce::String("Destroying instance UUID ")
			+ m_uuid.toDashedString());
#endif

	InstanceManager::get()->unregisterInstance(m_uuid);
}

//
// Setup & processing
//

void SuperSeparator::prepareToPlay(double sampleRate,
		int maximumExpectedSamplesPerBlock)
{
#ifdef SUPSEP_LOGGING
	DebugLog::log(m_logname, juce::String("prepareToPlay: ")
			+ juce::String(sampleRate) + ' '
			+ juce::String(maximumExpectedSamplesPerBlock));
#endif

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
}

void SuperSeparator::releaseResources()
{
#ifdef SUPSEP_LOGGING
	DebugLog::log(m_logname, "releaseResources");
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

	// Apply settings
	m_floatDelayLine.setDelay(static_cast<float>(m_paramDelay->get()));

	SampleType mainInputCoeff = 1;
	SampleType sideInputCoeff = -1;
	if (m_paramInvert->getIndex() == 1)
	{
		mainInputCoeff = -1;
		sideInputCoeff = 1;
	}

	// Grab input & output data pointers
	auto main = getBusBuffer(buffer, true, 0);
	auto side = getBusBuffer(buffer, true, 1);

	SampleType const ** pmain = main.getArrayOfReadPointers();
	SampleType const ** pside = side.getArrayOfReadPointers();
	SampleType ** dst = main.getArrayOfWritePointers();

	// Main processing
	for (int i = 0; i < buffer.getNumSamples(); ++i)
	{
		for (int j = 0; j < main.getNumChannels(); ++j)
		{
			delayLine.pushSample(j, pmain[j][i] * mainInputCoeff);
			dst[j][i] = delayLine.popSample(j) + pmain[j][i];
		}
	}

	for (int i = 0; i < buffer.getNumSamples(); ++i)
	{
		for (int j = 0; j < side.getNumChannels(); ++j)
		{
			delayLine.pushSample(j + 2, pside[j][i] * sideInputCoeff);
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
	DebugLog::log(m_logname, d, false);
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
	DebugLog::log(m_logname, d, false);
#endif
	processBlock(buffer, m_doubleDelayLine);
}

//
// State loading & saving
//

void SuperSeparator::getStateInformation(juce::MemoryBlock & destData)
{
	// Create an XML document describing the current parameter values,
	// serialise it to a string, then chuck it in the destination memory block
	juce::XmlElement settings("settings");

	// Settings version. Bump this if non-backwards-compatible changes are
	// ever made.
	settings.setAttribute("version", 1);

	// Delay time
	std::unique_ptr<juce::XmlElement> delay{new juce::XmlElement("delay")};
	delay->setAttribute("time", m_paramDelay->get());
	settings.addChildElement(delay.release());

	// Input inversion
	std::unique_ptr<juce::XmlElement> invert{new juce::XmlElement("invert")};
	invert->setAttribute("channel", m_paramInvert->getIndex());
	settings.addChildElement(invert.release());

	// Instance UUID
	std::unique_ptr<juce::XmlElement> uuid{new juce::XmlElement("uuid")};
	uuid->setAttribute("uuid", m_uuid.toString());

	// Serialise & copy to memory block
	copyXmlToBinary(settings, destData);

#ifdef SUPSEP_LOGGING
	DebugLog::log(m_logname, juce::String("getStateInformation:\n---\n")
			+ settings.toString() + juce::String("---"));
#endif
}

void SuperSeparator::setStateInformation(void const * data, int size)
{
	// Decode string to XML
	std::unique_ptr<juce::XmlElement> settings{getXmlFromBinary(data, size)};

#ifdef SUPSEP_LOGGING
	DebugLog::log(m_logname, juce::String("setStateInformation:\n---\n")
			+ settings->toString() + juce::String("---"));
#endif

	// Check settings version. If unsupported, just leave everything at default
	if (settings->getIntAttribute("version") != 1)
	{
#ifdef SUPSEP_LOGGING
		juce::String d("Unsupported settings version: ");
		d += settings->getIntAttribute("version");
		DebugLog::log(m_logname, d);
#endif
		return;
	}

	// Read settings & set parameter values. Leave unchanged at current values
	// if attributes are for some reason not present, but this shouldn't
	// happen.
	for (auto * e : settings->getChildIterator())
	{
		if (e->getTagName() == "delay")
		{
			int v = e->getIntAttribute("time", m_paramDelay->get());
			*m_paramDelay = v;
		}
		else if (e->getTagName() == "invert")
		{
			int v = e->getIntAttribute("channel", m_paramInvert->getIndex());
			*m_paramInvert = v;
		}
		else if (e->getTagName() == "uuid")
		{
			juce::Uuid old{m_uuid};
			juce::String v = e->getStringAttribute("uuid", m_uuid.toString());
			m_uuid = v;
#ifdef SUPSEP_LOGGING
			DebugLog::log(m_logname,
					juce::String("Old UUID: ") + m_uuid.toDashedString());
			m_logname = juce::String::toHexString(m_uuid.hash());
			DebugLog::log(m_logname,
					juce::String("New UUID: ") + m_uuid.toDashedString());
#endif
			if (old != m_uuid)
			{
				InstanceManager::get()->unregisterInstance(old);
				// TODO Pass Remote's pointer
				InstanceManager::get()->registerInstance(m_uuid, nullptr);
			}
		}
	}
}

//
// GUI
//

juce::AudioProcessorEditor * SuperSeparator::createEditor()
{
	return new Editor(this);
}
