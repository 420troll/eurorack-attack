/* Host integration for the separate Chakra Plaits SUB VST. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "params.h"

typedef struct AEffect AEffect;
typedef intptr_t (*cb)(AEffect*,int32_t,int32_t,intptr_t,void*,float);
struct AEffect { int32_t magic; intptr_t (*d)(AEffect*,int32_t,int32_t,intptr_t,void*,float);
 void*p; void (*setP)(AEffect*,int32_t,float); float (*getP)(AEffect*,int32_t);
 int32_t np,npar,ni,no,flags; intptr_t r1,r2; int32_t a,b,c; float io; void*obj,*user; int32_t uid,ver;
 void (*pr)(AEffect*,float**,float**,int32_t); void*pdr; char f[56]; };
typedef struct { int32_t type,byteSize,deltaFrames,flags,noteLength,noteOffset; unsigned char m[4]; char x[4]; } ME;
typedef struct { int32_t n; intptr_t r; void* ev[2]; } EV;
extern AEffect* VSTPluginMain(cb);

static intptr_t host(AEffect*e,int32_t op,int32_t i,intptr_t v,void*p,float o) {
    static double ti[16]; (void)e;(void)i;(void)v;(void)p;(void)o;
    if(op==7){ti[4]=120.0;((int32_t*)&ti[8])[5]=1<<10;return(intptr_t)ti;} return 0;
}
static int idx(const char*k){for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,k))return i;return-1;}
static void setv(AEffect*a,const char*k,float real){int i=idx(k);a->setP(a,i,(real-PARAMS[i].min)/(PARAMS[i].max-PARAMS[i].min));}
static void midi(AEffect*a,int s,int d1,int d2){ME m={1,sizeof(ME),0,0,0,0,{(unsigned char)s,(unsigned char)d1,(unsigned char)d2,0},{0}};EV e={1,0,{&m,0}};a->d(a,25,0,0,&e,0);}
static double run(AEffect*a,int blocks,double*stereo_error){float L[128],R[128],*o[2]={L,R};double s=0,e=0;for(int b=0;b<blocks;b++){a->pr(a,0,o,128);for(int i=0;i<128;i++){s+=L[i]*L[i]+R[i]*R[i];e+=fabs(L[i]-R[i]);}}if(stereo_error)*stereo_error=e;return sqrt(s/(blocks*256.0));}
static double freq(AEffect*a,int blocks){float L[128],R[128],*o[2]={L,R};int crossings=0,samples=0;float prev=0;for(int b=0;b<blocks;b++){a->pr(a,0,o,128);for(int i=0;i<128;i++){if(prev<=0&&L[i]>0)crossings++;prev=L[i];samples++;}}return crossings*44100.0/samples;}
static int fails;
#define CHECK(c,...) do{printf("%s ",(c)?"ok  ":"FAIL");printf(__VA_ARGS__);printf("\n");if(!(c))fails++;}while(0)

int main(void){
 AEffect*a=VSTPluginMain(host); const char*sub[]={"sub_division","sub_glide_ms","sub_fold","sub_mix","sub_harmonic_lock","sub_phase_lock","sub_enabled"};
 CHECK(a&&a->uid==0x54635362,"UID is TcSb"); CHECK(a&&a->ni==0&&a->no==2&&(a->flags&(1<<8)),"instrument has stereo output");
 for(int i=0;i<7;i++)CHECK(idx(sub[i])>=0,"registered %s",sub[i]);
 CHECK(fabs(a->getP(a,idx("sub_division"))-1.0f)<1e-6f&&fabs(a->getP(a,idx("sub_glide_ms"))-.18f)<1e-6f&&fabs(a->getP(a,idx("sub_fold"))-.2f)<1e-6f&&fabs(a->getP(a,idx("sub_mix"))-.5f)<1e-6f,"sub defaults are 10 / 900ms / 20%% / 50%%");
 CHECK(PARAMS[idx("model")].nopts==24,"all 24 models are registered");
 setv(a,"sub_enabled",0); setv(a,"volume",1); setv(a,"amp_mode",0);
 for(int m=0;m<24;m++){setv(a,"model",m);midi(a,0x90,60,100);double r=run(a,80,0);CHECK(isfinite(r)&&r>1e-6,"model %d renders",m);midi(a,0xB0,120,0);run(a,5,0);}
 setv(a,"volume",0);setv(a,"sub_enabled",1);setv(a,"sub_division",10);setv(a,"sub_glide_ms",0);setv(a,"sub_fold",0);setv(a,"sub_mix",.5f);
 midi(a,0x90,69,127);double se=0,mr=run(a,40,&se);CHECK(mr>1e-4&&se<1e-7,"MIDI triggers identical stereo sub output");CHECK(fabs(freq(a,400)-44.0)<1.2,"MIDI A4 sub is 44 Hz");
 midi(a,0x90,76,100);CHECK(fabs(freq(a,400)-65.41)<1.2,"newest MIDI note owns sub");midi(a,0x80,76,0);CHECK(fabs(freq(a,400)-44.0)<1.2,"note-off falls back to last held note");
 midi(a,0xB0,64,127);midi(a,0x80,69,0);CHECK(run(a,30,0)>1e-4,"sustain retains released MIDI owner");midi(a,0xB0,64,0);run(a,20,0);CHECK(run(a,40,0)<1e-5,"pedal-up releases deferred sub note");
 setv(a,"chakra_source",1);setv(a,"chakra_hz",528);setv(a,"chakra_gate",1);CHECK(fabs(freq(a,400)-52.8)<1.2,"HOLD triggers Chakra root");setv(a,"chakra_hz",880);CHECK(fabs(freq(a,400)-88.0)<1.2,"active HOLD root retunes");
 midi(a,0x90,69,100);CHECK(fabs(freq(a,400)-44.0)<1.2,"MIDI temporarily owns sub over HOLD");midi(a,0x80,69,0);CHECK(fabs(freq(a,400)-88.0)<1.2,"MIDI release falls back to HOLD");setv(a,"chakra_gate",0);run(a,20,0);CHECK(run(a,40,0)<1e-5,"HOLD release decays to silence");
 midi(a,0x90,60,100);setv(a,"chakra_gate",1);midi(a,0xB0,123,0);run(a,20,0);CHECK(a->getP(a,idx("chakra_gate"))<.5f&&run(a,40,0)<1e-5,"all-notes-off clears MIDI and HOLD owners");
 printf("%s\n",fails?"FAILED":"PASSED");a->d(a,1,0,0,0,0);return fails!=0;
}
