

#include "PluginProcessor.h"


_3BandEqualizerAudioProcessor::_3BandEqualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
#endif
	apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

_3BandEqualizerAudioProcessor::~_3BandEqualizerAudioProcessor()
{
}


const juce::String _3BandEqualizerAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool _3BandEqualizerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool _3BandEqualizerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool _3BandEqualizerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double _3BandEqualizerAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int _3BandEqualizerAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int _3BandEqualizerAudioProcessor::getCurrentProgram()
{
	return 0;
}

void _3BandEqualizerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String _3BandEqualizerAudioProcessor::getProgramName(int index)
{
	return {};
}

void _3BandEqualizerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}


void _3BandEqualizerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = 1;
	spec.sampleRate = sampleRate;

	leftChain.prepare(spec);
	rightChain.prepare(spec);

	auto chainSettings = getChainSettings(apvts); 
	updatePeakFilter(chainSettings);

	auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2 * (chainSettings.lowCutSlope + 1));

	auto& leftLowCut = leftChain.get<ChainIndex::LowCut>();
	auto& rightLowCut = rightChain.get<ChainIndex::LowCut>(); 
	
	updateCutFilter(leftLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);
	updateCutFilter(rightLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);

	auto highCutCoefficient = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,sampleRate, 2 * (chainSettings.highCutSlope + 1));

	auto& leftHighCut = leftChain.get<ChainIndex::HighCut>();
	auto& rightHighCut = rightChain.get<ChainIndex::HighCut>();

	updateCutFilter(leftHighCut, highCutCoefficient, (Slope)chainSettings.highCutSlope);
	updateCutFilter(rightHighCut, highCutCoefficient, (Slope)chainSettings.highCutSlope);
}

void _3BandEqualizerAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _3BandEqualizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void _3BandEqualizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	auto chainSettings = getChainSettings(apvts);

	updatePeakFilter(chainSettings);

	auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2 * (chainSettings.lowCutSlope + 1));

	auto& leftLowCut = leftChain.get<ChainIndex::LowCut>();
	auto& rightLowCut = rightChain.get<ChainIndex::LowCut>();

	updateCutFilter(leftLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);
	updateCutFilter(rightLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);

	auto highCutCoefficient = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2 * (chainSettings.highCutSlope + 1));

	auto& leftHighCut = leftChain.get<ChainIndex::HighCut>();
	auto& rightHighCut = rightChain.get<ChainIndex::HighCut>();

	updateCutFilter(leftHighCut, highCutCoefficient, (Slope)chainSettings.highCutSlope);
	updateCutFilter(rightHighCut, highCutCoefficient, (Slope)chainSettings.highCutSlope);

	juce::dsp::AudioBlock<float> block(buffer);

	auto leftBlock = block.getSingleChannelBlock(0);
	auto rightBlock = block.getSingleChannelBlock(1);

	juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
	juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

	leftChain.process(leftContext);
	rightChain.process(rightContext);
}


bool _3BandEqualizerAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* _3BandEqualizerAudioProcessor::createEditor()
{
	return new juce::GenericAudioProcessorEditor(*this);
}


void _3BandEqualizerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void _3BandEqualizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout _3BandEqualizerAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>("LowCutFreq", "LowCutFreq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("HighCutFreq", "HighCutFreq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("PeakFreq", "PeakFreq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 750.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("PeakGain", "PeakGain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("PeakQuality", "PeakQuality", juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

	juce::StringArray stringArray;
	for (int32_t i = 0; i < 4; ++i)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " db/Oct";
		stringArray.add(str);
	}

	layout.add(std::make_unique<juce::AudioParameterChoice>("LowCutSlope", "LowCutSlope", stringArray, 0.0f));
	layout.add(std::make_unique<juce::AudioParameterChoice>("HighCutSlope", "HighCutSlope", stringArray, 0.0f));

	return layout;
}

void _3BandEqualizerAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
	auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQuality, juce::Decibels::decibelsToGain(chainSettings.peakChainInDecibels));

	*leftChain.get<ChainIndex::Peak>().coefficients = *peakCoefficients;
	*rightChain.get<ChainIndex::Peak>().coefficients = *peakCoefficients;
}



// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new _3BandEqualizerAudioProcessor();
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainSettings settings;
	settings.lowCutFreq = apvts.getRawParameterValue("LowCutFreq")->load();
	settings.highCutFreq = apvts.getRawParameterValue("HighCutFreq")->load();
	settings.peakFreq = apvts.getRawParameterValue("PeakFreq")->load();
	settings.peakChainInDecibels =apvts.getRawParameterValue("PeakGain")->load();
	settings.peakQuality = apvts.getRawParameterValue("PeakQuality")->load();
	settings.lowCutSlope = apvts.getRawParameterValue("LowCutSlope")->load();
	settings.highCutSlope = apvts.getRawParameterValue("HighCutSlope")->load();
	return settings;
}
