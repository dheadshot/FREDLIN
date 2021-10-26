/* Fredlin Quick and Dirty Additional Functions */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "fqdaf.h"


long getmin(long l1, long l2)
{
  if (l1<l2) return l1; else return l2;
}


long getmax(long l1, long l2)
{
  if (l1>l2) return l1; else return l2;
}


int strltrim(char *outstring, char *instring)
{
  int charptr=0;
  int ocharptr=0;
  while (instring[charptr]==32) charptr++;
  while (instring[charptr]!=0)
  {
    outstring[ocharptr]=instring[charptr];
    charptr++;
    ocharptr++;
  }
  outstring[ocharptr]=0;
  return 0;
}


int isfredlinlndigit(int dig)
{
  if (isdigit(dig)) return 1;
  if (dig==35) return 1;
  if (dig == 46) return 1;
  return 0;
}

unsigned long sstrlen(char *astr)
{
  if (!astr) return 0;
  return strlen(astr);
}

int streq_(char *str1, char *str2)
{
  unsigned long l1=sstrlen(str1), l2=sstrlen(str2);
  if (l1 != l2) return 0;
  if (memcmp(str1, str2, l1)==0) return 1;
  return 0;
}

int nstreq(char *str1, char *str2)
{
  /*char str2a[256]="";
  strcpy(str2a,str2);
  strcat(str2a,"\n");
  if ((strcmp(str1,str2)==0) || (strcmp(str1,str2a)==0))
  {
    return 1;
  }
  return 0;*/ /* This method was limited to 256 characters! */
  if (streq_(str1,str2)) return 1;
  unsigned long l1 = sstrlen(str1), l2 = sstrlen(str2);
  if ((l1 == (l2 + 1) || l1 == (l2 + 2)) && 
      (str1[l1-1] == '\n' || str1[l1-1] == '\r') && 
      memcmp(str1, str2, l2) == 0)
    return 1;
  return 0;
}


int instr(char *targstring, char *findstring, int startoffset)
{
  int i=0;
  int j=0;
  int ans=-1;
  i+=startoffset;
  
  while (j>-1)
  {
    j=instrch(targstring, findstring[0], i);
    if (j>-1)
    {
      if (strncmp(targstring+j+i, findstring,strlen(findstring))==0)
      {
        ans = j+i;
        return ans;
      }
      i+=j+1;
    }
  }
  return ans;
}


int replacestronce(char *outstring, char *targstring, char *findstring, char *repstring, int maxlen, int startoffset)
{
  int i=0;
  int j=0;
  int ans=0;
  int a=0;
  if (strlen(targstring)-strlen(findstring)+strlen(repstring) > maxlen) return -2;
  i = instr(targstring,findstring,startoffset);
  if (i>-1)
  {
    for (j=0;j<i;j++)
    {
      outstring[j]=targstring[j];
    }
    for (j=0;j<strlen(repstring);j++)
    {
      outstring[j+i]=repstring[j];
    }
    a=i;
    a+=strlen(findstring);
    i+=strlen(repstring);
    ans=i;
    for (j=a;j<=strlen(targstring);j++)
    {
      outstring[j-a+i]=targstring[j];
    }
    return ans;
  }
  else
  {
    return -1;
  }
}


int firstletterpos(char *cmdstring)
{
  int i=0;
  while (cmdstring[i] != 0)
  {
    if (isalpha(cmdstring[i])) return i;
    i++;
  }
  return -1;
}


long getfredlinlnnum(char *astr, int startoffset)
{
  /*
    Returns:
    0+	=	Number
    -1	=	Current line
    -2	=	Line after last
    -3	=	No number
    -10	=	Invalid number
  */
  int i=0;
  char lnnum[256]="\x00";
  int j=0;
  i+=startoffset;
  while (astr[i] != 0)
  {
    if (isfredlinlndigit(astr[i]))
    {
      lnnum[j]=astr[i];
      j++;
    }
    else
    {
      break;
    }
    i++;
  }
  if (lnnum[0]==46)
  {
    return FLN_CURRENT;
  }
  if (lnnum[0]==35)
  {
    return FLN_AFTERLAST;
  }
  if (lnnum[0]==0)
  {
    return FLN_NONUMBER;
  }
  if (isdigit(lnnum[0]))
  {
    return atol(lnnum);
  }
  return FLN_INVALIDNUMBER;
}

void printdebug(char *astr)
{
  int i;
  printf(">>");
  for (i=0;astr[i]!=0;i++)
  {
    if (astr[i]<32 || astr[i]>126) printf("%03d ",astr[i]);
    else printf("'%c' ",astr[i]);
  }
  printf("\n");
  return;
}
