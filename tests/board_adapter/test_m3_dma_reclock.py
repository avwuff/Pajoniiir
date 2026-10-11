"""Execute M3 production reclocking with mocked I2S failure boundaries."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[2]
source=(ROOT/'firmware/boards/m3/bsp_p4_m3/bsp_p4_m3.c').read_text(encoding='utf-8')
start=source.index('esp_err_t bsp_audio_main_i2s_set_sample_rate(')
end=source.index('{',start)+1;depth=1
while depth:
    depth+=(source[end]=='{')-(source[end]=='}');end+=1
code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef int esp_err_t;
typedef struct { uint32_t rate; } i2s_std_clk_config_t;
#define CONFIG_BSP_PCM5102A_MAIN_OUT 1
#define ESP_OK 0
#define ESP_ERR_INVALID_STATE -1
#define ESP_RETURN_ON_ERROR(expr,tag,msg) do { int rc=(expr); if (rc) return rc; } while(0)
#define I2S_STD_CLK_DEFAULT_CONFIG(rate) ((i2s_std_clk_config_t){rate})
static void *s_i2s_tx_pcm5102=(void*)1;
static bool s_i2s_tx_pcm5102_enabled;
static uint32_t s_main_dma_last_us;
static int fail_step,calls;
static int i2s_channel_disable(void *h) { assert(h);return ++calls==fail_step ? -2:0; }
static int i2s_channel_reconfig_std_clock(void *h,const i2s_std_clk_config_t *cfg)
{ assert(h && cfg->rate==48000);return ++calls==fail_step ? -2:0; }
static int i2s_channel_enable(void *h)
{ assert(h && !s_i2s_tx_pcm5102_enabled && s_main_dma_last_us==0);return ++calls==fail_step ? -2:0; }
'''
code+=source[start:end]+r'''
int main(void) {
    s_i2s_tx_pcm5102_enabled=true;s_main_dma_last_us=123456;
    assert(bsp_audio_main_i2s_set_sample_rate(0)==ESP_ERR_INVALID_STATE && !calls);
    for (fail_step=0;fail_step<=3;fail_step++) {
        calls=0;s_i2s_tx_pcm5102_enabled=true;s_main_dma_last_us=123456;
        assert(bsp_audio_main_i2s_set_sample_rate(48000)==(fail_step ? -2:0));
        assert(s_i2s_tx_pcm5102_enabled==(fail_step==0 || fail_step==1));
        assert(s_main_dma_last_us==((fail_step==1 || fail_step==2) ? 123456u:0));
    }
}
'''
out=ROOT/'.cache/m3_dma_reclock_review';out.mkdir(parents=True,exist_ok=True)
cfile=out/'test.c';cfile.write_text(code,encoding='utf-8')
exe=out/('test.exe' if os.name=='nt' else 'test')
subprocess.run([shutil.which('gcc'),'-std=c11','-Wall','-Wextra','-Werror',str(cfile),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
print('PASS production M3 reclock: reset DMA interval before enable, preserve disable/clock/enable errors')
