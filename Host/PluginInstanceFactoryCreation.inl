[] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode); },
[] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode); },
[] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode); },
[] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode); },

[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<AudioPlayerAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<DelayAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<DistortionAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<FilterAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<FlangerAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<NoiseGateAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<OscillatorAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<PingPongDelayAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<ReverbAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<SimpleDistortionAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<SimpleEQAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<ThreeBandEqualizerAudioProcessor>()); },
[] { return std::make_unique<PluginInstanceProxy> (std::make_unique<ChorusAudioProcessor>()); },