class AudioSystem {
private:
    AudioDevice* current_device;
    List play_queue;
    uint32_t sample_rate;
    bool is_playing;

public:
    void initialize(AudioDevice* device) {
        current_device = device;
        sample_rate = 44100;
        is_playing = false;
    }

    void play_sound(AudioBuffer* buffer) {
        play_queue.push_back(buffer);
        if (!is_playing) {
            start_playback();
        }
    }

    void handle_interrupt() {
        if (!play_queue.empty()) {
            auto buffer = play_queue.front();
            current_device->write_samples(buffer->get_data(), buffer->get_size());
            if (buffer->is_finished()) {
                play_queue.pop_front();
            }
        } else {
            stop_playback();
        }
    }
};