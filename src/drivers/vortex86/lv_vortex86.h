#ifndef LV_VORTEX86_H
#define LV_VORTEX86_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../display/lv_display.h"
#include "../../indev/lv_indev.h"
#include "../../draw/lv_image_dsc.h"

#if LV_USE_VORTEX86

enum {
    LV_VORTEX86_INPUT_DEFAULT,
    LV_VORTEX86_INPUT_INCLUDE_KBMS
};

lv_display_t * lv_vortex86_vga_create(int32_t hor_res, int32_t ver_res, void * draw_buf, uint32_t buf_size_bytes);
lv_display_t * lv_vortex86_lcd_create(int32_t hor_res, int32_t ver_res, void * draw_buf, uint32_t buf_size_bytes);
void lv_vortex86_inputs_create(lv_display_t * disp, int additionInput, lv_image_dsc_t const * mouse_img);

#endif /* LV_USE_VORTEX86 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_VORTEX86_H */