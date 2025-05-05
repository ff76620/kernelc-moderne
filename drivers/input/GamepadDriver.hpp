class GamepadDriver {
private:
    struct GamepadState {
        uint16_t buttons;
        int8_t left_stick_x;
        int8_t left_stick_y;
        int8_t right_stick_x;
        int8_t right_stick_y;
        uint8_t left_trigger;
        uint8_t right_trigger;
    };

    GamepadState current_state;
    List listeners;

public:
    void update() {
        GamepadState new_state = read_hardware_state();
        if (state_changed(new_state)) {
            current_state = new_state;
            notify_listeners();
        }
    }

    void register_listener(GamepadEventListener* listener) {
        listeners.push_back(listener);
    }

    const GamepadState& get_state() const {
        return current_state;
    }
};