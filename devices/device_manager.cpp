
#include "kernel/devices/device_manager.hpp"
#include "kernel/drivers/DriverManager.hpp"

void DeviceManager::initialize() {
    // Détection initiale du matériel
    detectHardware();
    
    // Configuration du sous-système de périphériques
    setupDeviceSubsystem();
    
    // Chargement des pilotes
    DriverManager::loadDrivers();
    
    // Initialisation complète des périphériques
    initializeDevices();
    
    // Détection finale des périphériques
    detectDevices();
}

void DeviceManager::setupDeviceSubsystem() {
    // Configuration des bus système
    initializeSystemBuses();
    
    // Configuration des interruptions matérielles
    setupHardwareInterrupts();
}

void DeviceManager::initializeDevices() {
    // Configuration des périphériques d'entrée
    setupInputDevices();
    
    // Configuration des périphériques de stockage
    setupStorageDevices();
}
    