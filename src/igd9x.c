#include <stdio.h>
#include <string.h>
#include "../include/igd9x.h"

#define IGD9X_PITCH_ALIGN 64U
#define IGD9X_MAX_BPP 32U
#define IGD9X_MIN_BPP 8U

static int igd9x_is_supported_bpp(u16 bpp)
{
    switch (bpp) {
    case 8:
    case 16:
    case 24:
    case 32:
        return 1;
    default:
        return 0;
    }
}

u16 igd9x_calculate_pitch(u16 width, u16 bpp)
{
    u32 raw_bytes;
    u32 aligned;

    if (width == 0 || bpp == 0) {
        return 0;
    }

    raw_bytes = ((u32)width * (u32)bpp + 7U) / 8U;
    aligned = (raw_bytes + (IGD9X_PITCH_ALIGN - 1U)) & ~(IGD9X_PITCH_ALIGN - 1U);

    if (aligned > 0xFFFFUL) {
        return 0;
    }

    return (u16)aligned;
}

static int igd9x_mode_supported(const igd9x_mode_t *mode)
{
    if (mode == 0) {
        return 0;
    }
    if (mode->width == 0 || mode->height == 0) {
        return 0;
    }
    if (mode->bpp < IGD9X_MIN_BPP || mode->bpp > IGD9X_MAX_BPP) {
        return 0;
    }
    if (!igd9x_is_supported_bpp(mode->bpp)) {
        return 0;
    }
    if (mode->refresh_hz == 0) {
        return 0;
    }
    if (mode->pitch_bytes < igd9x_calculate_pitch(mode->width, mode->bpp)) {
        return 0;
    }
    if (mode->pixel_clock_khz == 0) {
        return 0;
    }
    if (mode->pixel_clock_khz > igd9x_get_mode_db()->max_pixel_clock_khz) {
        return 0;
    }
    return 1;
}

int igd9x_validate_mode(const igd9x_mode_t *mode)
{
    return igd9x_mode_supported(mode);
}

const igd9x_mode_t *igd9x_find_mode(u16 width, u16 height, u16 bpp, u16 refresh_hz)
{
    const igd9x_mode_db_t *db;
    const igd9x_mode_t *mode;
    u16 i;

    db = igd9x_get_mode_db();
    for (i = 0; i < db->mode_count; ++i) {
        mode = &db->modes[i];
        if (mode->width == width &&
            mode->height == height &&
            mode->bpp == bpp &&
            (refresh_hz == 0 || mode->refresh_hz == refresh_hz) &&
            igd9x_mode_supported(mode)) {
            return mode;
        }
    }

    return 0;
}

const igd9x_mode_t *igd9x_pick_best_mode(u16 width, u16 height, u16 bpp, u16 refresh_hz)
{
    const igd9x_mode_db_t *db;
    const igd9x_mode_t *best;
    const igd9x_mode_t *mode;
    u16 i;

    db = igd9x_get_mode_db();
    best = 0;

    for (i = 0; i < db->mode_count; ++i) {
        mode = &db->modes[i];

        if (!igd9x_mode_supported(mode)) {
            continue;
        }
        if (mode->bpp != bpp) {
            continue;
        }

        if (mode->width < width || mode->height < height) {
            continue;
        }
        if (refresh_hz != 0 && mode->refresh_hz != refresh_hz) {
            continue;
        }

        if (best == 0 ||
            mode->width < best->width ||
            (mode->width == best->width && mode->height < best->height) ||
            (mode->width == best->width && mode->height == best->height &&
             mode->pixel_clock_khz < best->pixel_clock_khz)) {
            best = mode;
        }
    }

    if (best != 0) {
        return best;
    }

    return igd9x_find_mode(width, height, bpp, refresh_hz);
}

const char *igd9x_mode_name(const igd9x_mode_t *mode, char *buffer, u16 buffer_len)
{
    char temp[32];
    unsigned int width;
    unsigned int height;
    unsigned int refresh_hz;
    unsigned int bpp;
    size_t len;

    if (mode == 0 || buffer == 0 || buffer_len == 0) {
        return 0;
    }

    width = (unsigned int)mode->width;
    height = (unsigned int)mode->height;
    refresh_hz = (unsigned int)mode->refresh_hz;
    bpp = (unsigned int)mode->bpp;

    sprintf(temp, "%ux%u@%u-%ubpp", width, height, refresh_hz, bpp);
    len = strlen(temp) + 1U;
    if (len > (size_t)buffer_len) {
        return 0;
    }
    memcpy(buffer, temp, len);
    return buffer;
}
