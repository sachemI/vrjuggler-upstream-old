#ifndef _GADGET_FASTRACK_STANDALONE_H_
#define _GADGET_FASTRACK_STANDALONE_H_

// driver R.E.

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <signal.h>

#define NSTATION 4
#define XYZ 3
// 3 first letters forced in lower case
#define LC3(x) (0x202020|(((x)[0]<<16)|((x)[1]<<8)|(x)[2]))
// 2 first letters forced in lower case
#define LC2(x) (0x2020|(((x)[0]<<8)|(x)[1]))

// C driver (R.E.) functions and procedures
extern "C" void trackerInit(char *configfile);
extern "C" void getNewCoords(unsigned int station, float *vecXYZ, float *vecAER);
extern "C" int getCoords(unsigned int stations, float *vecXYZ, float *vecAER);
extern "C" int getNewButtonStatus(unsigned int station);
extern "C" void trackerFinish();

// bits positions of bits in flag word "config.found"
enum conf { DEV, BAUD, BUTTON,
	    REC, REC1, REC2, REC3,
	    TIP, TIP1, TIP2, TIP3,
	    INC, INC1, INC2, INC3,
	    HEM, HEM1, HEM2, HEM3,
	    ARF, ARF1, ARF2, ARF3,
	    TMF, TMF1, TMF2, TMF3,
};

// each reportable thing is identified by a bit in config.perstation[n].rec
// the following enum defines the bit positions
enum rec { Pos, Ang, Quat, But };

static int trackerfd;

static volatile unsigned char TrackerBuf[256];
static volatile int DoFlush;
static pid_t ReadPid;

struct perstation {
  int begin;
  int posoff;
  int angoff;
  int quatoff;
  int butoff;
  int rec;
  float tip[XYZ];
  float inc;
  float hem[XYZ];
  float arf[3*XYZ];
  float tmf[XYZ];
};

static
struct {
  int found;         // flags: one bit for each feature found in the chunk
  int len;           // total length of a message sent by the tracker
  char port[20];     // port the tracker is attached to
  int baud;          // port speed
  char button;
  char cont;
  struct perstation perstation[NSTATION];
} conf;

// end driver R.E.


#endif /* _GADGET_FASTRACK_STANDALONE_H_ */
