#include "kernel/filesystem/fs_manager.hpp"
#include "kernel/filesystem/vfs.hpp"

void FilesystemManager::initialize() {
    // Initialisation du système de fichiers
    setupFilesystems();
    
    // Configuration du VFS
    VFS::initialize();
    
    // Montage des systèmes de fichiers
    mountFilesystems();
}

void FilesystemManager::setupFilesystems() {
    // Configuration des systèmes de fichiers
    initializeFileSystemDrivers();
    
    // Configuration du cache
    setupFileCache();
}
    