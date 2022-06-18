# Copyright 2022 Philip Allison
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <https://www.gnu.org/licenses/>. 

# Could probably be lower, but it's what I have, and I'm unfamiliar with CMake
cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

project(SuperSeparator VERSION 0.1.0)

add_subdirectory(subprojects/JUCE)

juce_add_plugin(supsep
	COMPANY_NAME "Depthbuffer"
	BUNDLE_ID "uk.depthbuffer.supsep"
	DESCRIPTION "Super Separator"
	IS_SYNTH FALSE
	FORMATS VST3
	VST3_CATEGORIES Fx
	PRODUCT_NAME "Super Separator"
)

include_directories(src/)
aux_source_directory(src/ sources)
target_sources(supsep PRIVATE ${sources})
target_compile_definitions(supsep PUBLIC
	JUCE_DISPLAY_SPLASH_SCREEN=0
	JUCE_REPORT_APP_USAGE=0
	DONT_SET_USING_JUCE_NAMESPACE=1
	# As JUCE no longer embeds the VST2 SDK, but there has never been a VST2
    # version of this plugin to be backwards compatible with, disable backwards
	# compatibility options so we can build VST3 without requiring the VST2 SDK
	JUCE_VST3_CAN_REPLACE_VST2=0
	$<$<CONFIG:Debug>:SUPSEP_LOGGING=1>
)
target_link_libraries(supsep PUBLIC
	juce::juce_recommended_warning_flags
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
)
target_link_libraries(supsep PRIVATE
	juce::juce_audio_plugin_client
	juce::juce_dsp
)
juce_generate_juce_header(supsep)
