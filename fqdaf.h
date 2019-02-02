#ifndef __INC_FQDAF_H__
#define __INC_FQDAF_H__ 1

/* Fredlin Quick and Dirty Additional Functions */

#define FLN_CURRENT -1
#define FLN_AFTERLAST -2
#define FLN_NONUMBER -3
#define FLN_INVALIDNUMBER -10

long getmin(long l1, long l2);
long getmax(long l1, long l2);
int strltrim(char *outstring, char *instring);
int isfredlinlndigit(int dig);
unsigned long sstrlen(char *astr);
int streq_(char *str1, char *str2);
int nstreq(char *str1, char *str2);
int instr(char *targstring, char *findstring, int startoffset);
int replacestronce(char *outstring, char *targstring, char *findstring, char *repstring, int maxlen, int startoffset);
int firstletterpos(char *cmdstring);
long getfredlinlnnum(char *astr, int startoffset);

/* If qdinp2 is an installed library, change the speech marks to angle brackets! */
#include "qdinp2.h"
#define instrch(targstring, findchar, startoffset) qdinstrch(targstring, findchar, startoffset)

#endif
