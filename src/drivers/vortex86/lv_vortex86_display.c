#include "lv_vortex86.h"

#if LV_USE_VORTEX86
#include "../src/display/lv_display_private.h"
#include "drivers/vortex86/a9160.h"
#include "drivers/vortex86/lcd.h"
#include <sys/farptr.h>

#if (LV_COLOR_DEPTH == 8)
    #define PIXEL_SIZE 1
#elif (LV_COLOR_DEPTH == 16)
    #define PIXEL_SIZE 2
#elif (LV_COLOR_DEPTH == 32)
    #define PIXEL_SIZE 4
#endif

static void fbFastWrite(int sr, unsigned long offset, void *img, size_t img_size) {
    unsigned long addr = offset;
    int nb, nd, n;
    char *buf = (char *)img;
    size_t len = img_size;
    
    if (sr == -1) return;
    
    if (len > 64) {
        n = addr & 0x03;
        if (n > 0) {
            n = 4 - n;
            _farsetsel(sr);
            switch (n) {
            case 3: _farnspokeb(addr++, *buf++);
            case 2: _farnspokeb(addr++, *buf++);
            case 1: _farnspokeb(addr++, *buf++);
            }
            len -= n;
        }
    }

    nd = len >> 2;
    nb = len & 0x03;
    
    if (nd > 0) {
        n = (nd + 15) >> 4;
        addr = addr - (((0 - nd) & 15) << 2);
        buf = buf - (((0 - nd) & 15) << 2);
        _farsetsel(sr);
        switch (nd & 15) {
        case 0: do { _farnspokel(addr + 0x00, ((unsigned long*)buf)[0x00]);
        case 15:     _farnspokel(addr + 0x04, ((unsigned long*)buf)[0x01]);
        case 14:     _farnspokel(addr + 0x08, ((unsigned long*)buf)[0x02]);
        case 13:     _farnspokel(addr + 0x0C, ((unsigned long*)buf)[0x03]);
        case 12:     _farnspokel(addr + 0x10, ((unsigned long*)buf)[0x04]);
        case 11:     _farnspokel(addr + 0x14, ((unsigned long*)buf)[0x05]);
        case 10:     _farnspokel(addr + 0x18, ((unsigned long*)buf)[0x06]);
        case 9:      _farnspokel(addr + 0x1C, ((unsigned long*)buf)[0x07]);
        case 8:      _farnspokel(addr + 0x20, ((unsigned long*)buf)[0x08]);
        case 7:      _farnspokel(addr + 0x24, ((unsigned long*)buf)[0x09]);
        case 6:      _farnspokel(addr + 0x28, ((unsigned long*)buf)[0x0A]);
        case 5:      _farnspokel(addr + 0x2C, ((unsigned long*)buf)[0x0B]);
        case 4:      _farnspokel(addr + 0x30, ((unsigned long*)buf)[0x0C]);
        case 3:      _farnspokel(addr + 0x34, ((unsigned long*)buf)[0x0D]);
        case 2:      _farnspokel(addr + 0x38, ((unsigned long*)buf)[0x0E]);
        case 1:      _farnspokel(addr + 0x3C, ((unsigned long*)buf)[0x0F]);
                     addr += 0x40;
                     buf += 0x40;
                } while (--n > 0);
        }
    }
    
    switch (nb) {
    case 3: _farsetsel(sr);
            _farnspokew(addr, ((unsigned short*)buf)[0]);
            _farnspokeb(addr + 2, buf[2]);
            return;
    case 2: _farpokew(sr, addr, ((unsigned short*)buf)[0]);
            return;
    case 1: _farpokeb(sr, addr, buf[0]);
    }
}

void vortex86_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    unsigned long area_x, draw_size, screen_width_size, offset, height;
    int *selector = (int*)lv_display_get_user_data(disp);

    lv_color16_t *color_p = (lv_color16_t *)px_map;
    
    if (area->x1 == 0 && area->x2 == disp->hor_res) {
        draw_size = disp->hor_res * (area->y2 - area->y1 + 1);
        offset = area->y1 * (disp->hor_res * PIXEL_SIZE);
        
        fbFastWrite(*selector, offset, color_p, draw_size);
    } else {
        screen_width_size = disp->hor_res * PIXEL_SIZE;
        area_x = area->x2 - area->x1 + 1;
        draw_size = area_x * PIXEL_SIZE;
        offset = area->y1 * screen_width_size + area->x1 * PIXEL_SIZE;
        height = area->y2 - area->y1 + 1;
        
        for (; height; height--) {
            fbFastWrite(*selector, offset, color_p, draw_size);
            offset += screen_width_size;
            color_p += area_x;
        }
    }
    
    lv_display_flush_ready(disp); /* tell lvgl that flushing is done */
}

lv_display_t * lv_vortex86_vga_create(int32_t hor_res, int32_t ver_res, void *draw_buf, uint32_t buf_size_bytes) {
    static int selector = -1;
    
    a9160_Init(hor_res, ver_res);
    selector = get_a9160_fb_selector();
        
    lv_display_t *disp = lv_display_create(hor_res, ver_res);
    lv_display_set_flush_cb(disp, vortex86_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, buf_size_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(disp, (void*)&selector);
    
    return disp;
}

lv_display_t * lv_vortex86_lcd_create(int32_t hor_res, int32_t ver_res, void * draw_buf, uint32_t buf_size_bytes) {
    static int selector = -1;
    
    if (hor_res == 1280 && ver_res == 1024)
        lcd_on(0);
    else if (hor_res == 1024 && ver_res == 600)
        lcd_on(1);
    else if (hor_res == 800 && ver_res == 480)
        lcd_on(2);
    else if (hor_res == 600 && ver_res == 480)
        lcd_on(3);
    else {
        return NULL;
    }
    
    selector = get_lcd_fb_selector();
        
    lv_display_t *disp = lv_display_create(hor_res, ver_res);
    lv_display_set_flush_cb(disp, vortex86_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, buf_size_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(disp, (void*)&selector);
    
    return disp;
}

#endif /*LV_USE_VORTEX86*/
