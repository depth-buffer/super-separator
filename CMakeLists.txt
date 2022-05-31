# Could probably be lower, but it's what I have, and I'm unfamiliar with CMake
cmake_minimum_required(VERSION 3.23)

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
)
target_link_libraries(supsep PUBLIC
	juce::juce_recommended_warning_flags
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
)
target_link_libraries(supsep PRIVATE
	juce::juce_audio_plugin_client
)
juce_generate_juce_header(supsep)
