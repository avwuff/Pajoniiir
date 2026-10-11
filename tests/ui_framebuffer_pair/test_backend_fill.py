"""Execute the production RGB565 fill on a paired framebuffer with HAL stubs."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / 'firmware/p4-core/components/ui/ui_lvgl_backend.c').read_text(encoding='utf-8')
start = source.rindex('esp_err_t ui_lvgl_backend_draw_rect_rgb565(')
opening = source.index('{', start)
depth, end = 1, opening + 1
while depth:
    depth += (source[end] == '{') - (source[end] == '}')
    end += 1
function = source[start:end]
code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define UI_DSI_FB_COUNT 2u
#define BSP_LCD_H_RES 8
#define BSP_LCD_V_RES 4
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_INVALID_STATE 2
#define ESP_CACHE_MSYNC_FLAG_DIR_C2M 1
#define ESP_CACHE_MSYNC_FLAG_UNALIGNED 2
typedef int esp_err_t;
typedef struct { int x,y,w,h; } ui_overlay_rect_t;
typedef struct { unsigned scanout_bytes_per_pixel; } board_t;
static const board_t board = {2};
static const board_t *board_capabilities_get(void) { return &board; }
static uint16_t pixels[32];
static void *s_dsi_fb[2] = {pixels, NULL};
static int s_dsi_active_fb_idx, s_framebuffers, dirty_calls, sync_calls;
static unsigned s_hor_res=8, s_ver_res=4;
static bool ui_board_map(ui_overlay_rect_t in, unsigned w, unsigned h, ui_overlay_rect_t *out)
{ if(in.x<0 || in.y<0 || in.x+in.w>(int)w || in.y+in.h>(int)h) return false; *out=in; return true; }
static bool bsp_scanout_fill_rect_rgb565(void *fb, size_t n, unsigned w, unsigned h,
    int x, int y, int rw, int rh, uint16_t c)
{ (void)fb;(void)n;(void)w;(void)h;(void)x;(void)y;(void)rw;(void)rh;(void)c;assert(false);return false; }
static bool ui_framebuffer_pair_dirty(void *pair, int x, int y, int w, int h)
{ assert(pair==&s_framebuffers && x==2 && y==1 && w==3 && h==2); dirty_calls++;return true; }
static esp_err_t esp_cache_msync(void *p, size_t n, unsigned flags)
{ assert(p==pixels+8 && n==32 && flags==3 && dirty_calls==sync_calls+1);sync_calls++;return 77; }
'''
code += function + r'''
int main(void) {
    const ui_overlay_rect_t rect={2,1,3,2}, invalid={7,1,3,2};
    assert(ui_lvgl_backend_draw_rect_rgb565(NULL, 1)==ESP_ERR_INVALID_ARG);
    assert(ui_lvgl_backend_draw_rect_rgb565(&invalid, 1)==ESP_ERR_INVALID_ARG);
    assert(dirty_calls==0 && sync_calls==0);
    assert(ui_lvgl_backend_draw_rect_rgb565(&rect, 0x1234)==77);
    for(unsigned y=0;y<4;y++) for(unsigned x=0;x<8;x++)
        assert(pixels[y*8+x]==((y>=1 && y<3 && x>=2 && x<5)?0x1234:0));
    assert(dirty_calls==1 && sync_calls==1);
    s_dsi_active_fb_idx=1;
    assert(ui_lvgl_backend_draw_rect_rgb565(&rect, 1)==ESP_ERR_INVALID_STATE);
    assert(dirty_calls==1 && sync_calls==1);
}
'''
out = root / '.cache/ui_backend_fill_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out / 'test.c'
cfile.write_text(code, encoding='utf-8')
exe = out / ('test.exe' if os.name == 'nt' else 'test')
gcc = shutil.which('gcc') or 'C:/msys64/ucrt64/bin/gcc.exe'
subprocess.run([gcc, '-std=c11', '-Wall', '-Wextra', '-Werror', '-Wno-error=sign-compare',
                str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('PASS production RGB565 fill: bounded pixels, paired dirty region, cache-error propagation')
