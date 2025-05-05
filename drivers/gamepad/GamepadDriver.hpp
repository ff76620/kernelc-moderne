class GamepadDriver {
private:
    struct GamepadState {
        uint16_t buttons;
        int16_t axes[4];
        uint8_t triggers[2];
    };

    GamepadState current_state;
    bool connected;
    uint8_t player_id;

public:
    void initialize(uint8_t id) {
        player_id = id;
        connected = false;
        reset_state();
        setup_usb_endpoint();
    }

    void process_input_report(const uint8_t* report, size_t length) {
        if (length >= sizeof(GamepadState)) {
            memcpy(¤t_state, report, sizeof(GamepadState));
            notify_input_handlers();
        }
    }

    bool is_button_pressed(GamepadButton button) {
        return (current_state.buttons & (1 << static_cast(button))) != 0;
    }

    float get_axis_value(GamepadAxis axis) {
        if (axis < 4) {
            return current_state.axes[axis] / 32768.0f;
        }
        return 0.0f;
    }
};