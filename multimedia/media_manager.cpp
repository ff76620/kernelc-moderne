
#include "kernel/multimedia/media_manager.hpp"
#include "kernel/multimedia/codec_manager.hpp"

void MediaManager::initialize() {
    // Initialisation du système multimédia
    setupMediaSubsystem();
    
    // Chargement des codecs
    CodecManager::loadCodecs();
    
    // Configuration du pipeline média
    setupMediaPipeline();
}

void MediaManager::setupMediaSubsystem() {
    // Configuration des périphériques audio
    initializeAudioDevices();
    
    // Configuration des périphériques vidéo
    setupVideoDevices();
}