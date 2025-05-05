#ifndef KERNEL_TYPES_HPP
#define KERNEL_TYPES_HPP

#include 
#include 

namespace kernel {

// Basic integer types
typedef int8_t          int8;
typedef uint8_t         uint8;
typedef int16_t         int16;
typedef uint16_t        uint16;
typedef int32_t         int32;
typedef uint32_t        uint32;
typedef int64_t         int64;
typedef uint64_t        uint64;

// Process and thread identifiers
typedef int32          pid_t;
typedef int32          tid_t;

// File system types
typedef uint64         inode_t;
typedef uint32         mode_t;
typedef int32          fd_t;        // File descriptor
typedef uint64         offset_t;    // File offset
typedef uint32         dev_t;       // Device ID

// Memory management
typedef uint64         paddr_t;     // Physical address
typedef uint64         vaddr_t;     // Virtual address
typedef uint64         size_t;      // Size type
typedef int64         ssize_t;      // Signed size type

// Time related
typedef uint64         time_t;      // Time type
typedef uint64         nsec_t;      // Nanoseconds
typedef uint64         usec_t;      // Microseconds
typedef uint64         msec_t;      // Milliseconds

// Network types
typedef uint32         ipv4_t;
typedef uint16         port_t;
typedef uint64         socket_t;

// Device management
typedef uint32         device_id_t;
typedef uint32         driver_id_t;

// Audio types
typedef uint32         sample_t;
typedef uint32         format_t;
typedef uint32         channel_t;

// Graphics types
typedef uint32         color_t;
typedef uint32         pixel_t;
typedef uint32         resolution_t;

// Status and error handling
typedef int32          status_t;
typedef int32          error_t;

// Security and permissions
typedef uint32         uid_t;       // User ID
typedef uint32         gid_t;       // Group ID
typedef uint32         perm_t;      // Permission flags

// Resource identifiers
typedef uint32         resource_id_t;
typedef uint32         handle_t;

// Event and signal types
typedef uint32         event_t;
typedef uint32         signal_t;

// Power management
typedef uint32         power_state_t;
typedef uint32         thermal_zone_t;

// Generic flags and options
typedef uint32         flags_t;
typedef uint32         options_t;

// Media and codec types
typedef uint32         codec_id_t;
typedef uint32         stream_id_t;
typedef uint32         buffer_id_t;

// Input device types
typedef uint32         input_id_t;
typedef uint32         key_code_t;
typedef uint32         scan_code_t;

// Common structures
struct Point {
    int32 x;
    int32 y;
};

struct Rectangle {
    int32 x;
    int32 y;
    uint32 width;
    uint32 height;
};

struct TimeSpec {
    time_t seconds;
    nsec_t nanoseconds;
};

struct Version {
    uint16 major;
    uint16 minor;
    uint16 patch;
};

// Common constants
static const status_t STATUS_SUCCESS = 0;
static const status_t STATUS_ERROR = -1;
static const handle_t INVALID_HANDLE = static_cast(-1);
static const fd_t INVALID_FD = -1;

} // namespace kernel

#endif // KERNEL_TYPES_HPP