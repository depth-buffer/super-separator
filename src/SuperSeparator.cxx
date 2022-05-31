#include <JuceHeader.h>

#include "SuperSeparator.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuperSeparator();
}
