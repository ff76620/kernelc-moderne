#include "kernel/drivers/audio/audio_manager.hpp"
#include "kernel/drivers/audio/codec_manager.hpp"

void AudioManager::initialize() {
    // Initialisation du matériel audio
    detectAudioDevices();
    
    // Configuration des codecs
    CodecManager::initialize();
    
    // Initialisation du mixeur
    initializeMixer();
}

void AudioManager::initializeMixer() {
    // Configuration des canaux audio
    setupAudioChannels();
    
    // Initialisation du contrôle du volume
    setupVolumeControl();
}