cmake_minimum_required(VERSION 3.22)
project(Sonarium VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.16-security GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Sonarium
    COMPANY_NAME "ThomC"
    BUNDLE_ID "com.thomc.sonarium"
    PLUGIN_MANUFACTURER_CODE Thmc
    PLUGIN_CODE Sona
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    FORMATS VST3 Standalone
    COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "Sonarium")
juce_generate_juce_header(Sonarium)
target_sources(Sonarium PRIVATE
    Source/PluginProcessor.h Source/PluginProcessor.cpp
    Source/PluginEditor.h Source/PluginEditor.cpp
    Source/SonariumDSP.h)
target_compile_definitions(Sonarium PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0)
target_link_libraries(Sonarium PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

enable_testing()
add_executable(sonarium_dsp_stress Tests/DSPStress.cpp)
target_include_directories(sonarium_dsp_stress PRIVATE Source)
target_compile_features(sonarium_dsp_stress PRIVATE cxx_std_17)
if (MSVC)
    target_compile_options(sonarium_dsp_stress PRIVATE /W4 /EHsc)
else()
    target_compile_options(sonarium_dsp_stress PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME SonariumDSPStress COMMAND sonarium_dsp_stress)
