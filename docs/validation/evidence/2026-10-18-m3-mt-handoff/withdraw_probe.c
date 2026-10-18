
#include "audio_keylock.h"
#include "audio_resampler.h"
#include "audio_pcm_timeline.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
static int16_t pcm[192000*2];
static audio_pcm_timeline_t timeline;
static audio_keylock_t keylock;
static unsigned reads, failures, max_reads, rewinds, source_empty;
static bool read_frame(void *ctx,uint64_t seq,audio_mixer_frame_t *out) {
 (void)ctx; ++reads; return audio_pcm_timeline_read_output_owner(&timeline,seq,out);
}
static bool pop_frame(void *ctx,audio_mixer_frame_t *out) {
 (void)ctx;bool ok=audio_pcm_timeline_pop(&timeline,out);if(!ok)++source_empty;return ok;
}
int main(void) {
 unsigned cases=0; clock_t started=clock();
 for(unsigned r=0;r<2;++r) for(unsigned tempo=0;tempo<2;++tempo)
 for(unsigned phase=0;phase<256;phase+=85) for(unsigned mode=0;mode<7;++mode) {
  const unsigned keeps[]={0,1,32,256,2048,4800,94000};
  audio_pcm_timeline_init(&timeline,pcm,192000);
  for(unsigned n=0;n<150000;++n) assert(audio_pcm_timeline_push(&timeline,(n*97u)%24000,(n*71u)%24000));
  audio_keylock_reset(&keylock,0); keylock.antialias=true;
  float ratio=r?1.0f:44100.f/48000.f;
  audio_keylock_configure(&keylock,tempo?1.05f:.95f,ratio);
  audio_resampler_state_t rs={0};rs.antialias=true;
  for(unsigned n=0;n<8192+phase;++n) {
   audio_mixer_frame_t out;uint64_t seq=audio_pcm_timeline_play_seq(&timeline);uint32_t consumed;
   assert(audio_keylock_next(&keylock,read_frame,0,&out,&consumed,&seq));
   assert(audio_pcm_timeline_set_playhead_output_owner(&timeline,seq));
  }
  unsigned future=audio_pcm_timeline_future_frames(&timeline),keep=keeps[mode];
  if(keep>future)keep=future;
  audio_pcm_timeline_drop_newest(&timeline,future-keep);
  for(unsigned n=0;n<1024;++n) {
   audio_mixer_frame_t out;uint64_t before=audio_pcm_timeline_play_seq(&timeline),seq=before;uint32_t consumed;
   reads=0;
   bool ok=audio_keylock_next(&keylock,read_frame,0,&out,&consumed,&seq);
   if(ok)ok=audio_pcm_timeline_set_playhead_output_owner(&timeline,seq);
   if(!ok){++failures;out=audio_resampler_next(&rs,ratio*(tempo?1.05f:.95f),pop_frame,0,&consumed);}
   if(audio_pcm_timeline_play_seq(&timeline)<before)++rewinds;
   if(reads>max_reads)max_reads=reads;
   if(n==512)for(unsigned j=0;j<5000;++j)assert(audio_pcm_timeline_push(&timeline,1200,-1200));
  }
  ++cases;
 }
 printf("cases=%u keylock_fallback_frames=%u empty_source_reads=%u backward_playhead_moves=%u max_source_reads_per_frame=%u cpu_seconds=%.3f\n",cases,failures,source_empty,rewinds,max_reads,(double)(clock()-started)/CLOCKS_PER_SEC);
 return 0;
}
