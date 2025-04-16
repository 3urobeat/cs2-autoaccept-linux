#ifndef _RENDER_H
#define _RENDER_H

#include <pixman.h>
#include <stdbool.h>

#include "grim.h"

#ifdef __cplusplus
extern "C" {
#endif

bool is_format_supported(enum wl_shm_format fmt);
uint32_t get_format_min_stride(enum wl_shm_format fmt, uint32_t width);

pixman_image_t *render(struct grim_state *state, struct grim_box *geometry,
	double scale);

#ifdef __cplusplus
}
#endif

#endif
