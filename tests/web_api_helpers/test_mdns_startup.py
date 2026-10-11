"""Execute optional mDNS startup and web teardown from production source."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root/'firmware/p4-core/components/web_server/web_server.c').read_text(encoding='utf-8')

def function(signature):
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
static bool s_mdns_started;
static void *s_web_server;
static int fail_step, calls, frees, stops;
static int step(void) { return ++calls == fail_step ? -1 : ESP_OK; }
static int mdns_init(void) { return step(); }
static int mdns_hostname_set(const char *s) { (void)s; return step(); }
static int mdns_instance_name_set(const char *s) { (void)s; return step(); }
static int mdns_service_add(const char *n, const char *s, const char *p, int port, void *txt, int count)
{ (void)n;(void)s;(void)p;(void)port;(void)txt;(void)count;return step(); }
static void mdns_free(void) { frees++; }
static void httpd_stop(void *server) { assert(server);stops++; }
'''
code += function('static void web_server_start_mdns(void)')
code += function('void web_server_stop(void)')
code += r'''
int main(void) {
    for (fail_step=0; fail_step<=4; fail_step++) {
        calls=frees=stops=0;s_mdns_started=false;s_web_server=(void*)1;
        web_server_start_mdns();
        assert(s_web_server && !stops);
        assert(calls==(fail_step ? fail_step : 4));
        assert(s_mdns_started==(fail_step==0));
        assert(frees==(fail_step>1 ? 1 : 0));
        web_server_stop();
        assert(!s_web_server && !s_mdns_started && stops==1);
        assert(frees==(fail_step==1 ? 0 : 1));
        web_server_stop();assert(stops==1);
    }
}
'''
out = root/'.cache/mdns_startup_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code, encoding='utf-8')
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('PASS production mDNS startup: all failures preserve HTTP; owned cleanup and idempotent stop')
