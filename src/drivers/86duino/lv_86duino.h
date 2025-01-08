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

#if LV_USE_86DUINO

enum {
    LV_86DUINO_INPUT_AUTO,
    LV_86DUINO_INPUT_ONLY_TOUCHPAD,
    LV_86DUINO_INPUT_ONLY_KBMS,
    LV_86DUINO_INPUT_TOUCHPAD_KBMS,
};

lv_display_t * lv_86duino_display_create(void * draw_buf, uint32_t buf_size_bytes);
void lv_86duino_inputs_create(lv_display_t * disp, int inputType, lv_image_dsc_t const * mouse_img);

#endif /* LV_USE_VORTEX86 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_VORTEX86_H */