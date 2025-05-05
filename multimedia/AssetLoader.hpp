
// Dans kernel/multimedia/AssetLoader.hpp
class AssetLoader {
private:
    ThreadPool threadPool;
    std::queue> loadingQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    bool isRunning = true;

public:
    AssetLoader(size_t threadCount = 4) : threadPool(threadCount) {
        startWorkers();
    }

    template
    std::future loadAssetAsync(const std::string& path) {
        return threadPool.enqueue([path]() {
            return loadAsset(path);
        });
    }

    void queueAssetLoad(const std::string& path, 
                       std::function callback) {
        std::lock_guard lock(queueMutex);
        loadingQueue.push([=]() {
            void* asset = loadAssetRaw(path);
            callback(asset);
        });
        queueCV.notify_one();
    }

private:
    void startWorkers() {
        for (size_t i = 0; i < threadPool.getThreadCount(); ++i) {
            threadPool.enqueue([this]() {
                processingLoop();
            });
        }
    }

    void processingLoop() {
        while (isRunning) {
            std::function task;
            {
                std::unique_lock lock(queueMutex);
                queueCV.wait(lock, [this]() {
                    return !loadingQueue.empty() || !isRunning;
                });
                
                if (!isRunning) return;
                
                task = std::move(loadingQueue.front());
                loadingQueue.pop();
            }
            task();
        }
    }

    template
    static AssetType loadAsset(const std::string& path) {
        // Implémentation spécifique selon le type d'asset
        return AssetType();
    }

    static void* loadAssetRaw(const std::string& path) {
        // Implémentation du chargement générique
        return nullptr;
    }
};