/* CPU Throttle utility for VIA 686A/B by Lesha Bogdanow. This code is free.
   NO WARRANTY. Use at your own risk.

   To be compiled with GCC/EMX 0.9d 
 */

#define INCL_BASE

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os2.h>
#include <sys\hw.h>

unsigned long pmbase;
int verbose=0;

APIRET APIENTRY DosPerfSysCall(ULONG ulCommand, ULONG ulParm1,
                                 ULONG ulParm2, ULONG ulParm3);

  #define ORD_DOS32PERFSYSCALL       976
  #define CMD_KI_RDCNT           	(0x63)

  typedef struct _CPUUTIL {
    ULONG ulTimeLow;     /* Low 32 bits of time stamp      */
    ULONG ulTimeHigh;    /* High 32 bits of time stamp     */
    ULONG ulIdleLow;     /* Low 32 bits of idle time       */
    ULONG ulIdleHigh;    /* High 32 bits of idle time      */
    ULONG ulBusyLow;     /* Low 32 bits of busy time       */
    ULONG ulBusyHigh;    /* High 32 bits of busy time      */
    ULONG ulIntrLow;     /* Low 32 bits of interrupt time  */
    ULONG ulIntrHigh;    /* High 32 bits of interrupt time */
   } CPUUTIL;

  typedef CPUUTIL *PCPUUTIL;

  /* Convert 8-byte (low, high) time value to double */
  #define LL2F(high, low) (4294967296.0*(high)+(low))

/* Return CPU load (0-100) or -1 if DosPerfSysCall() failed
   First result is 0 and should be discarded */
int CPULoad() {
   static int iter=0;
   static double ts_val_prev, idle_val_prev;
   APIRET      rc;
   int         ret;
   double      ts_val, idle_val;
   CPUUTIL     CPUUtil;

   rc = DosPerfSysCall(CMD_KI_RDCNT,(ULONG) &CPUUtil,0,0);
   if (rc) return -1;
   ts_val = LL2F(CPUUtil.ulTimeHigh, CPUUtil.ulTimeLow);
   idle_val = LL2F(CPUUtil.ulIdleHigh, CPUUtil.ulIdleLow);

   if (iter) {
      double  ts_delta = ts_val - ts_val_prev;
      ret=100-(int)((idle_val - idle_val_prev)/ts_delta*100.0);
      if (ret<0) ret=0;
      if (ret>100) ret=100;
      }
   else {
      iter++;
      ret=0;
      }
   ts_val_prev = ts_val;
   idle_val_prev = idle_val;
   return ret;
   }

void PCICfgOpen() {
   _portaccess(0xCF8,0xCFF);
   }
unsigned long PCICfgRead(unsigned long bus, unsigned long dev,
                unsigned long fn, unsigned long reg) {
   _outp32(0xCF8,0x80000000|(bus<<16)|(dev<<11)|(fn<<8)|reg);
   return _inp32(0xCFC);
   }
void PCICfgWrite(unsigned long bus, unsigned long dev,
                unsigned long fn, unsigned long reg, unsigned long val) {
   _outp32(0xCF8,0x80000000|(bus<<16)|(dev<<11)|(fn<<8)|reg);
   _outp32(0xCFC,val);
   }
void PCICfgClose() {
   _outp32(0xCF8,0);
   }

void Help() {
   printf("\nUSAGE: THROTTLE {<Percentage>|OFF}\n");
   printf("- Set specified CPU speed and exit.\n");
   printf("<Percentage> - fraction of the CPU speed, 100 or OFF means the full speed.\n");
   printf("\nOR:    THROTTLE -h <HThres> -l <LThres> [-st <Time>] [-tt <Time>] <Percentage>\n");
   printf("- Automatic mode. The program keeps running and monitors CPU load.\n");
   printf("  When the load goes below lower threshold the CPU is throttled to the\n");
   printf("  specified fraction of its speed. When the load goes above the higher\n");
   printf("  threshold the CPU is set to its full speed.\n");
   printf("-h <HThres> - higher threshold of CPU load (%%), default 90\n");
   printf("-l <LThres> - lower threshold of CPU load (%%), default 10\n");
   printf("-st <Time>  - sleep time (in ms, 100 or above), default 60000 (1 minute)\n");
   printf("-tt <Time>  - sleep time (in ms, 100 or above) for throttled mode, default 500\n");
   exit(255);
   }

int OpenThrottle() {
   unsigned long dev,bus;

   PCICfgOpen();
   pmbase=0;
   if (verbose>=3) fprintf(stderr,"Searching PCI configuration space.\n");
   for (bus=0;!pmbase&&(bus<256);bus++) for(dev=0;!pmbase&&(dev<32);dev++) {
       if (PCICfgRead(bus,dev,0,0)==0x06861106)
          pmbase=PCICfgRead(bus,dev,4,0x48)&0xFF80;
       }
   if (verbose>=3) fprintf(stderr,"Closing PCI configuration access.\n");
   PCICfgClose();
   if (!pmbase) return 0;
   if (verbose) fprintf(stderr,"Found VIA 686A/B at bus %X device %X.\n",bus,dev);
   _portaccess(pmbase,pmbase+0x7F);
   return 1;
   }

void SetThrottle(int ratio) {
   unsigned long val;

   ratio=(ratio*15)/100+1;
   if (ratio<2) ratio=2;	// Avoid freezing.
   if (verbose>=3) fprintf(stderr,"Converted throttle ratio: %X.\n",ratio);
   val=_inp32(pmbase+0x10);
   if (verbose>=3) fprintf(stderr,"Old CPU and PCI PM control register value: %08X.\n",val);
   val&=0xFFFFFFE0;
   if (ratio<=15) val|=0x10|(unsigned long)ratio;
   if (verbose>=3) fprintf(stderr,"New CPU and PCI PM control register value: %08X.\n",val);
   _outp32(pmbase+0x10,val);
   return;
   }

int main(int argc, char *argv[]) {
   int ratio=-1,i;
   int HThres=-1,LThres=-1;
   int SleepTime=60000,SleepTimeT=500;
   int throttled=0,load;
   

   printf("CPU Throttle v.0.02 for VIA 686A/B by Lesha Bogdanow\n");
   printf("NO WARRANTY. Use at your own risk.\n");

   for (i=1;i<argc;i++) {
      if (!stricmp(argv[i],"-h"))  {
         i++;
         if (i==argc) {
            Help();
            exit(255);
            }
         HThres=atoi(argv[i]);
         }
      else if (!stricmp(argv[i],"-l"))  {
         i++;
         if (i==argc) Help();
         LThres=atoi(argv[i]);
         }
      else if (!stricmp(argv[i],"-st"))  {
         i++;
         if (i==argc) Help();
         SleepTime=atoi(argv[i]);
         }
      else if (!stricmp(argv[i],"-tt"))  {
         i++;
         if (i==argc) Help();
         SleepTimeT=atoi(argv[i]);
         }
      else if (!stricmp(argv[i],"-v")) verbose++;
      else if (!stricmp(argv[i],"OFF")) ratio=100;
      else ratio=atoi(argv[i]);
      }
   if ((ratio<0)||(ratio>100)) Help();
   if ((LThres>=0)||(HThres>=0)) {
      if (LThres<0) LThres=10;
      if (HThres<0) HThres=90;
      if ((ratio==100)||(LThres>=HThres)||(LThres>100)||(HThres>100)||
         (SleepTime<100)||(SleepTimeT<100)) Help();
      }

   if (verbose) fprintf(stderr,"Ratio: %d, LThres: %d, HThres: %d\n",ratio,HThres, LThres);

   if (!OpenThrottle()) {
      printf("No VIA 686A/B south bridge found.\n");
      exit(1);
      }
   if (HThres<0) {
      SetThrottle(ratio);
      if (verbose) fprintf(stderr,"All done, exiting.");
      exit(0);
      }
/* "Resident" mode */
   CPULoad();			// Dummy call
   do {
      if (verbose>=2) fprintf(stderr,"Sleeping ... \n",load);
      if (!throttled) DosSleep(SleepTime);
      else DosSleep(SleepTimeT);
      load=CPULoad();
      if (verbose>=2) fprintf(stderr,"CPU load: %d",load);
      if (throttled&&(load>=HThres)) {
         if (verbose>=2) fprintf(stderr," - switching to full speed.");
         else if (verbose) fprintf(stderr,"Switching to full speed.\n");
         SetThrottle(100);
         throttled=0;
         }
      if (!throttled&&(load<=LThres)) {
         if (verbose>=2) fprintf(stderr," - throttling.");
         else if (verbose) fprintf(stderr,"Throttling.\n");
         SetThrottle(ratio);
         throttled=1;
         }
      if (verbose>=2) fprintf(stderr,"\n",load);
      } while (1);
   }
