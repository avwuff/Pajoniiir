"""Execute the production diagnostic limiter with remote API failures/readback."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class TxLimit(unittest.TestCase):
    def test_actual_limiter(self):
        source = (ROOT / 'firmware/p4-core/components/wifi_link/wifi_link.c').read_text()
        fn = source[source.index('static esp_err_t apply_diagnostic_tx_limit'):source.index('static esp_err_t start_web_ap')]
        c = r'''
#include <stdint.h>
#include <assert.h>
#define ESP_OK 0
#define ESP_ERR_INVALID_RESPONSE 9
#define ESP_RETURN_ON_ERROR(expr,...) do { int r=(expr);if(r)return r; }while(0)
#define SERVICE_LOG_WIFI_TX_POWER 1
#define SERVICE_LOG_INFO 0
#define SERVICE_LOG_ERROR 1
typedef int esp_err_t;
static int calls, fail_at, mismatch, logs;
static int __attribute__((unused)) esp_wifi_get_max_tx_power(int8_t *p) {
 calls++; if(calls==fail_at)return 7; *p=calls==1?80:(mismatch?20:8);return 0;
}
static int __attribute__((unused)) esp_wifi_set_max_tx_power(int8_t p) {
 calls++; assert(p==8);return calls==fail_at?7:0;
}
static void __attribute__((unused)) service_log_event(int e,int sev,int n,uint32_t a,uint32_t b,uint32_t c,uint32_t d,const char *s) {
 (void)e;(void)d;(void)s;assert(n==3 && a==80 && b==8);assert(c==(mismatch?20u:8u));assert(sev==mismatch);logs++;
}
''' + fn + r'''
int main(void) {
#if CONFIG_WIFI_LINK_DIAGNOSTIC_LOW_TX_POWER
 assert(apply_diagnostic_tx_limit()==0 && calls==3 && logs==1);
 for(int i=1;i<=3;i++){calls=logs=0;fail_at=i;assert(apply_diagnostic_tx_limit()==7 && calls==i && logs==0);}
 calls=logs=fail_at=0;mismatch=1;assert(apply_diagnostic_tx_limit()==9 && logs==1);
#else
 assert(apply_diagnostic_tx_limit()==0 && calls==0 && logs==0);
#endif
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as d:
            src=pathlib.Path(d)/'tx.c';src.write_text(c)
            for enabled in (0,1):
                exe=pathlib.Path(d)/f'tx{enabled}.exe'
                subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror',f'-DCONFIG_WIFI_LINK_DIAGNOSTIC_LOW_TX_POWER={enabled}',str(src),'-o',str(exe)],check=True)
                subprocess.run([str(exe)],check=True)


if __name__ == '__main__':
    unittest.main()
