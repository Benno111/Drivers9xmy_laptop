#ifndef IGD9X_H
#define IGD9X_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

typedef struct igd9x_mode {
    u16 width;
    u16 height;
    u16 bpp;
    u16 refresh_hz;
    u32 pixel_clock_khz;
    u16 htotal;
    u16 hsync_start;
    u16 hsync_end;
    u16 vtotal;
    u16 vsync_start;
    u16 vsync_end;
    u16 pitch_bytes;
    u8 interlaced;
    u8 double_scanned;
    u8 preferred;
} igd9x_mode_t;

typedef struct igd9x_mode_db {
    const igd9x_mode_t *modes;
    u16 mode_count;
    u16 native_width;
    u16 native_height;
    u16 native_bpp;
    u16 native_refresh_hz;
    u32 max_pixel_clock_khz;
} igd9x_mode_db_t;

typedef struct igd9x_hw_info {
    u16 vendor_id;
    u16 device_id;
    u8 bus;
    u8 device;
    u8 function;
    u32 class_code;
} igd9x_hw_info_t;

typedef enum igd9x_status {
    IGD9X_STATUS_OK = 0,
    IGD9X_STATUS_NOT_FOUND = 1,
    IGD9X_STATUS_UNSUPPORTED = 2,
    IGD9X_STATUS_INVALID_ARGUMENT = 3,
    IGD9X_STATUS_HW_FAILURE = 4
} igd9x_status_t;

const igd9x_mode_db_t *igd9x_get_mode_db(void);
const igd9x_mode_t *igd9x_get_mode_by_index(u16 index);
const igd9x_mode_t *igd9x_find_mode(u16 width, u16 height, u16 bpp, u16 refresh_hz);
const igd9x_mode_t *igd9x_pick_best_mode(u16 width, u16 height, u16 bpp, u16 refresh_hz);

u16 igd9x_calculate_pitch(u16 width, u16 bpp);
int igd9x_validate_mode(const igd9x_mode_t *mode);
const char *igd9x_mode_name(const igd9x_mode_t *mode, char *buffer, u16 buffer_len);

igd9x_status_t igd9x_hw_probe(const igd9x_hw_info_t *info);
igd9x_status_t igd9x_hw_set_mode(const igd9x_mode_t *mode);

#ifdef __cplusplus
}
#endif

#endif

