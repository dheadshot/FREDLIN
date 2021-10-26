#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "qdinp2.h"
#include "fqdaf.h"

#ifdef SIGINTTOCANCEL
#define EOFISCANCEL 0
#else
#define EOFISCANCEL 1
#endif

char progver[] = "0.61.02 BETA";
char progcopyright[] = "2015-2017 DHeadshot's Software Creations, 2017-2021 The FREDLIN Project";
char proghelpfile[] = "frhelp.txt";

struct flnode {
  char flline[256];
  struct flnode *next;
};




int printhelp(char *progname)
{
  printf("FREDLIN %s\n", progver);
  printf("%s\n", progcopyright);
  printf("Using the Quick and Dirty Input Library version %s\n  (see https://github.com/dheadshot/libqdinp2 ).\n",qdinpver());
  printf("Usage:\n");
  printf("  %s { --help | --test | <filename> }\n", progname);
  printf("--help	Print this message and end\n");
  printf("--test	Enter Input-Test-Mode\n");
  printf("<filename>	File to edit\n");
#ifndef NOHELP
  printf("\nType \"H\" for further help within the program or refer to the file \"%s\".\n", proghelpfile);
#else
  printf("\nRefer to the file \"%s\" for further help.\n", proghelpfile);
#endif
  
  return 0;
}



int savefile(struct flnode *rootnode, char *afn)
{
  FILE *thefile;
  struct flnode *flnptr;
  
  thefile = fopen(afn,"w");
  if (thefile==NULL) return errno;
  flnptr = rootnode->next;
  while (flnptr != 0)
  {
    if (fputs(flnptr->flline, thefile)==EOF)
    {
      fclose(thefile);
      return -1;
    }
    flnptr = flnptr->next;
  }
  fclose(thefile);
  return 0;
}


int main( int argc, char *argv[] )
{
  char afn[256]="";
  char bakfn[256]="";
  int fexists = 0;
  getterm();
  if ((argc != 2) || (streq_(argv[1],"--help")) || (streq_(argv[1],"-?")) || (streq_(argv[1],"--version")))
  {
    printhelp(argv[0]);
    return 0;
  }
  
  if ((streq_(argv[1],"-t")) || (streq_(argv[1],"--test")))
  {
    char linestore[256] = "";
    char linetemplate[256] = "The Quick Brown Fox Jumps Over The Lazy Dog!";
    readqdline(linestore, linetemplate,EOFISCANCEL);
    printf("\n\"%s\"\n",linestore);
    
    return 0;
  }
  if (argv[1][0]==45)
  {
    printf("Bad flag - use \"--help\" to see valid arguments!\n");
    return 1;
  }
  if (strlen(argv[1])>251) /* Must be <=251 to make .BAK file <=255 */
  {
    printf("File name too long--Rename file!\n");
    return 1;
  }
  
  strcpy(afn,argv[1]);
  strcpy(bakfn,afn);
  strcat(bakfn,".bak");
  if (access(afn,F_OK) != -1)
  {
    /* File Exists */
    if (rename(afn,bakfn)==-1)
    {
      printf("File Backup Failed!\n");
      return 2;
    }
    fexists = 1;
  }
  /* At this point, afn does not exist, but if bakfn does we need to read it in*/
  
  /* Let's build the linked-list first */
  struct flnode *root; /* root node */
  struct flnode *nodeptr; /* Points to nodes as we go through the list */
  
  struct flnode *tnodeptr; /* Temporary node pointer, for deletions etc */
  struct flnode *insnodeptr; /* Node pointer for inserts */
  struct flnode *movenodeptr; /* Node pointer for moves */
  struct flnode *copyroot; /* root node for copies */
  struct flnode *ttnodeptr; /* Another temporary node pointer */
  
  root = (struct flnode *) malloc( sizeof(struct flnode) ); /* Build the node */
  if (root == 0)
  {
    printf("Out of memory!\n");
    return 3;
  }
  root->next = 0; /* Only one in list, so point to NULL */
  clearstring(root->flline, 256);
  
  FILE *fp;
  char readstring[256]="";
  if (fexists == 1)
  {
    if (!(fp = fopen(bakfn,"r")))
    {
      /* Can't open file */
      printf("Error opening file!\n");
      if (rename(bakfn,afn)==-1)
      {
        printf("File Rename Failed!\n");
        return 2;
      }
      return 2;
    }
    nodeptr = root;
    /* Root stays blank so we can insert before the first line */
    while ( fgets(readstring,256,fp) != NULL )
    {
      nodeptr->next = (struct flnode *) malloc( sizeof(struct flnode) );
      nodeptr = nodeptr->next;
      if (nodeptr == 0)
      {
        printf("Out of memory!\n");
        return 3;
      }
      /* Load in text */
      strcpy(nodeptr->flline,readstring);
      nodeptr->next = 0;
    }
    if (ferror(fp))
    {
      printf("Error reading file!\n");
      fclose(fp);
      return 2;
    }
    /*
    nodeptr->next = (struct flnode *) malloc( sizeof(struct flnode) );
    nodeptr = nodeptr->next;
    if (nodeptr == 0)
    {
      printf("Out of memory!\n");
      return 3;
    }
    /* Load in text *
    strcpy(nodeptr->flline,readstring);
    nodeptr->next = 0;
    */
    fclose(fp);
    
    /* File is in List */
  }
  else
  {
    /* New File */
    printf("New file.\n");
  }
  
  /*--------------------------------------*/
  
  char lastcmd[256]="";
  char acmd[256]="";
  long curline=1;
  char exitmsg[] = "Abort edit?  [Y/N]";
  char exiterrmsg[] = "Do you still wish to exit? [Y/N]";
  char searchcorrectmsg[] = "Is this the correct occurence? [Y/N]";
  char repcorrectmsg[] = "Is this replacement correct? [Y/N]";
  char enterr[] = "Entry Error!\n";
  long rangestart = 0;
  long rangeend = 0;
  long moveline = 0;
  long copytimes = 0;
  long nlines = 0; //User feedback variable.
  int searchcheck = 0;
  int searchfound = 0;
  int tempi=0;
  long templ=0;
  char aline[256] = "";
  char bline[256] = "";
  char cline[256] = ""; /* For replacements */
  int ret = 0;
  
  while (1==1)
  {
    if ((ret = readqdline(acmd,lastcmd,EOFISCANCEL)) == 0)
    {
      strcpy(lastcmd,acmd);
      tempi=instrch(lastcmd,10,0);
      if (tempi>-1) lastcmd[tempi]=0;
      else printf("\n"); /* Ensure we're working on a new line */
      strltrim(aline,acmd);
      strcpy(acmd,aline);
      
      /* Do Command */
      /* Exit */
      if ((nstreq(acmd,"e")==1) || (nstreq(acmd,"E")==1))
      {
        tempi = savefile(root,afn);
        if (tempi==0)
        {
          printf("File written successfully to disk.\n");
          break;
        }
        switch (tempi)
        {
          case -1:
            printf("Error writing to file!\n");
          break;
          
          default:
            printf("Error %d opening file!\n",tempi);
          break;
        }
        if (yesnomsg(exiterrmsg)==1) break;
      }
      
      /* Quit */
      if ((nstreq(acmd,"q")==1) || (nstreq(acmd,"Q")==1))
      {
        if (yesnomsg(exitmsg)==1) 
        {
          if ((fexists==1) && (access(afn,F_OK)==-1))
          {
            if (rename(bakfn,afn)==-1)
            {
              printf("File Rename Failed!\n");
            }
          }
          break;
        }
      }
      else
      {
        switch (firstletter(acmd))
        {
          case (64+16):
          case (96+16):
            /* Page */
            if (firstletterpos(acmd)==0)
            {
              rangestart=1;
              rangeend=-2;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if (rangestart == FLN_CURRENT) rangestart = curline;
              if (rangestart == FLN_NONUMBER) rangestart = 1;
              tempi = instrch(acmd,44,0);
              if (tempi==-1) tempi = instrch(acmd,45,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
              if (rangeend==FLN_NONUMBER) rangeend = -2;
            }
            if ((rangestart == FLN_INVALIDNUMBER) || (rangestart == 0) 
               || (rangestart == FLN_AFTERLAST) || 
               (rangeend == FLN_INVALIDNUMBER) || (rangeend == 0) || 
               ((rangeend != FLN_AFTERLAST) && (rangestart > rangeend)))
            {
              printf(enterr);
            }
            else
            {
              /* List the nodes */
              nodeptr = root;
              templ=0;
              while (templ<rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              if (templ<rangestart)
              {
                printf("No lines in range!\n");
              }
              else
              {
                for (templ=rangestart; templ!=rangeend+1; templ++)
                {
                  if (nodeptr == 0) break;
                  printf("  %ld	",templ);
                  if (nodeptr->next==0) printf("*"); else printf(" ");
                  printf(":%s",nodeptr->flline);
                  if (instrch(nodeptr->flline,10,0)==-1) printf("\n-");
                  nodeptr = nodeptr->next;
                }
              }
              /* Set current line to range end */
              curline = templ - 1;
            }
          break;
          
          case (64+12):
          case (96+12):
            /* List */
            if (firstletterpos(acmd)==0)
            {
              rangestart = 1;
              rangeend = 23;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if ((rangestart == FLN_CURRENT) || (rangestart==FLN_NONUMBER)) 
                rangestart = curline;
              tempi = instrch(acmd,44,0);
              if (tempi==-1) tempi = instrch(acmd,45,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
            }
            if ((rangestart==FLN_INVALIDNUMBER) || 
                (rangeend==FLN_INVALIDNUMBER) || (rangestart==FLN_AFTERLAST) 
                || (rangestart==0) || (rangeend==0) || 
                ((rangeend != FLN_AFTERLAST) && (rangeend != FLN_NONUMBER) && 
                  (rangeend<rangestart)))
            {
              printf(enterr);
            }
            else
            {
              if (rangeend==FLN_NONUMBER)
              {
                rangestart = getmax(1,rangestart-11);
                rangeend=rangestart+22;
              }
              /* List the nodes */
              nodeptr = root;
              templ=0;
              while (templ<rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              if (templ<rangestart)
              {
                printf("No lines in range!\n");
              }
              else
              {
                for (templ=rangestart;templ!=rangeend+1;templ++)
                {
                  if (nodeptr == 0) break;
                  printf("  %ld	",templ);
                  if (curline==templ) printf("*"); else printf(" ");
                  printf(":%s",nodeptr->flline);
                  if (instrch(nodeptr->flline,10,0)==-1) printf("\n-");
                  nodeptr = nodeptr->next;
                }
                /*printf("\n");*/
              }
            }
          break;
          
          case 0:
            /* Edit line */
            rangestart = getfredlinlnnum(acmd,0);
            if (acmd[0]==0) rangestart = curline+1;
            switch (rangestart)
            {
              case FLN_CURRENT:
                rangestart = curline;
              break;
              
              case FLN_NONUMBER:
                rangestart = curline + 1;
              break;
              
              case FLN_AFTERLAST:
              case FLN_INVALIDNUMBER:
              case 0:
                printf(enterr);
                rangestart = FLN_INVALIDNUMBER;
              break;
            }
            if (rangestart>0)
            {
              /* Find the right node */
              nodeptr = root;
              templ=0;
              while (templ<rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              if (templ<rangestart)
              {
                printf(enterr);
              }
              else
              {
                /* Edit Line */
                curline = rangestart;
                printf("  %ld\t*",curline);
                printf(":%s",nodeptr->flline);
                if (instrch(nodeptr->flline,10,0)==-1) printf("\n-");
                printf("  %ld\t*:",curline);
                strcpy(bline,nodeptr->flline);
                tempi = instrch(bline,10,0);
                if (tempi>=0) bline[tempi] = 0;
                if (readqdline(aline,bline,EOFISCANCEL)==0)
                {
                  strcpy(nodeptr->flline,aline);
                  if (instrch(nodeptr->flline,10,0)==-1) printf("\n-");
                  printf("\n");
                }
                else
                {
                  printf("\nCancelled!\n");
                }
              }
            }
          break;
          
          case (64+23):
          case (96+23):
            /* Write File */
            /* Will just be a straight save - not [NUMBER] Writes here... */
            tempi = savefile(root,afn);
            switch (tempi)
            {
              case 0:
                printf("File written successfully to disk.\n");
              break;
              
              case -1:
                printf("Error writing to file!\n");
              break;
              
              default:
                printf("Error %d opening file!\n",tempi);
              break;
            }
          break;
          
          case (64+9):
          case (96+9):
            /* Insert Line */
            rangestart = getfredlinlnnum(acmd,0);
            switch (rangestart)
            {
              case FLN_CURRENT:
              case FLN_NONUMBER:
                rangestart = curline;
              break;
              
              case 0:
              case FLN_INVALIDNUMBER:
                rangestart = FLN_INVALIDNUMBER;
                printf(enterr);
              break;
            }
            if (rangestart != FLN_INVALIDNUMBER)
            {
              /* Find the right node */
              nodeptr = root;
              templ=01;
              while (templ!=rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              clearstring(bline,256);
              /* Read the lines and insert */
              tnodeptr = nodeptr->next;
              printf("  %ld\t*:",templ);
              while (readqdline(aline,bline,EOFISCANCEL)==0)
              {
                strcpy(bline,aline);
                tempi = instrch(bline,10,0);
                if (tempi>=0) bline[tempi] = 0;
                else printf("\n-"); /* Ensure we continue on a new line, but mark as continued. */
                insnodeptr = (struct flnode *) malloc( sizeof(struct flnode) ); /* Build the new node */
                if (insnodeptr==0)
                {
                  printf("Memory full!\n");
                  break;
                }
                strcpy(insnodeptr->flline,aline);
                insnodeptr->next = 0;
                nodeptr->next = insnodeptr;
                nodeptr = nodeptr->next;
                templ++;
                printf("  %ld\t*:",templ);
              }
              printf("\nCancelled!\n");
              nodeptr->next = tnodeptr;
              curline = templ-1;
            }
          break;
          
          case (64+4):
          case (96+4):
            /* Delete lines */
            if (firstletterpos(acmd)==0)
            {
              rangestart=curline;
              rangeend=curline;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if ((rangestart == FLN_CURRENT) || (rangestart == FLN_NONUMBER)) 
                rangestart = curline;
              tempi = instrch(acmd,44,0);
              if (tempi==-1) tempi = instrch(acmd,45,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
              if (rangeend==FLN_NONUMBER) rangeend = rangestart;
            }
            if ((rangestart == FLN_INVALIDNUMBER) || (rangestart == 0) || 
                (rangestart == FLN_AFTERLAST) || 
                (rangeend == FLN_INVALIDNUMBER) || (rangeend == 0) || 
                ((rangeend != FLN_AFTERLAST) && (rangestart > rangeend)))
            {
              printf(enterr);
            }
            else
            {
              nodeptr = root;
              templ=0;
              while (templ<rangestart-1)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              if (templ<rangestart-1) /* Never happens at the moment */
              {
                printf("No lines in range!\n");
              }
              else
              {
                /* Delete the lines */
                insnodeptr = nodeptr;
                tnodeptr = insnodeptr->next;
                nodeptr = nodeptr->next;
                for (templ=rangestart; templ!=rangeend+1; templ++)
                {
                  if (nodeptr == 0) break;
                  tnodeptr = nodeptr->next;
                  free(nodeptr);
                  nodeptr = tnodeptr;
                }
                insnodeptr->next = tnodeptr;
                nodeptr = insnodeptr;
                if (tnodeptr==0) curline=templ-2; else curline=templ-1;
                printf("Deleted %ld line(s)\n",(templ-rangestart));
              }
            }
          break;
          
          case (64+19):
          case (96+19):
            /* Search */
            if (firstletterpos(acmd)==0)
            {
              rangestart=1;
              rangeend=FLN_AFTERLAST;
              searchcheck=0;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if (rangestart == FLN_CURRENT) rangestart = curline;
              if (rangestart == FLN_NONUMBER) rangestart = 1;
              tempi = instrch(acmd,44,0);
              if (tempi==-1) tempi = instrch(acmd,45,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
              if (rangeend==FLN_NONUMBER) rangeend = FLN_AFTERLAST;
              if (acmd[firstletterpos(acmd)-1]==63) searchcheck = 1; 
              else searchcheck = 0;
            }
            strcpy(aline, acmd+firstletterpos(acmd)+1);
            tempi=instrch(aline,10,0);
            if (tempi>=0) aline[tempi]=0;
            tempi=instrch(aline,12,0);
            if (tempi>=0) aline[tempi]=10;
            printf("'%s'\n",aline);
            if (strlen(aline)<1)
            {
              printf("No Search String Specified!\n");
            }
            else
            {
              if ((rangestart == FLN_INVALIDNUMBER) || (rangestart == 0) || 
                  (rangestart == FLN_AFTERLAST) || 
                  (rangeend == FLN_INVALIDNUMBER) || (rangeend == 0) || 
                  ((rangeend != FLN_AFTERLAST) && (rangestart > rangeend)))
              {
                printf(enterr);
              }
              else
              {
                /* Find the right node */
                nodeptr = root;
                templ=0;
                while (templ<rangestart)
                {
                  if (nodeptr->next == 0) break;
                  nodeptr = nodeptr->next;
                  templ++;
                }
                if (templ<rangestart)
                {
                  printf(enterr);
                }
                else
                {
                  /* Do Search */
                  searchfound = 0;
                  for (templ=rangestart;templ!=rangeend;templ++)
                  {
                    tempi = 0;
                    tempi = instr(nodeptr->flline,aline,tempi);
LoopFind: /* This is a bit of a bodge */
                    if (tempi>=0)
                    {
                      printf("  %ld\t",templ);
                      if (templ==curline) printf("*"); else printf(" ");
                      printf(":%s",nodeptr->flline);
                      if (instrch(nodeptr->flline,10,0)==-1) printf("\n-");
                      if (searchcheck==1)
                      {
                        if (yesnomsg(searchcorrectmsg)!=0)
                        {
                          curline = templ;
                          searchfound=1;
                          break;
                        }
                      }
                      else
                      {
                        curline = templ;
                        searchfound=1;
                        break;
                      }
                    }
                    if (tempi>=0)
                    {
                      tempi++;
                      tempi = instr(nodeptr->flline,aline,tempi);
                      if (tempi>=0) goto LoopFind;
                    }
                    
                    if (nodeptr->next == 0) break;
                    nodeptr = nodeptr->next;
                  }
                  if (searchfound==0) printf("String not found within range\n");
                }
              }
            }
          break;
          
          case (64+18):
          case (96+18):
            /* Replace */
            if (firstletterpos(acmd)==0)
            {
              rangestart=1;
              rangeend=FLN_AFTERLAST;
              searchcheck=0;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if (rangestart == FLN_CURRENT) rangestart = curline;
              if (rangestart == FLN_NONUMBER) rangestart = 1;
              tempi = instrch(acmd,44,0);
              if (tempi==-1) tempi = instrch(acmd,45,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
              if (rangeend==FLN_NONUMBER) rangeend = FLN_AFTERLAST;
              if (acmd[firstletterpos(acmd)-1]==63) searchcheck = 1; 
              else searchcheck = 0;
            }
            strcpy(aline, acmd+firstletterpos(acmd)+1);
            tempi=instrch(aline,10,0);
            if (tempi>=0) aline[tempi]=0;
            if (strlen(aline)<1)
            {
              printf("No Search String Specified!\n");
            }
            else
            {
              printf("Enter replacement text:\n");
              if (readqdline(bline,aline, EOFISCANCEL)==0)
              {
                tempi=instrch(bline,10,0);
                if (tempi>=0) bline[tempi]=0;
                tempi=instrch(aline,12,0);
                if (tempi>=0) aline[tempi]=10; /* Moved here so template editing works */
                tempi=instrch(bline,12,0);
                if (tempi>=0) bline[tempi]=10;
                
                if ((rangestart == FLN_INVALIDNUMBER) || (rangestart == 0) || 
                    (rangestart == FLN_AFTERLAST) || 
                    (rangeend == FLN_INVALIDNUMBER) || (rangeend == 0) || 
                    ((rangeend != FLN_AFTERLAST) && (rangestart > rangeend)))
                {
                  printf(enterr);
                }
                else
                {
                  /* Find the right node */
                  nodeptr = root;
                  templ=0;
                  while (templ<rangestart)
                  {
                    if (nodeptr->next == 0) break;
                    nodeptr = nodeptr->next;
                    templ++;
                  }
                  if (templ<rangestart)
                  {
                    printf(enterr);
                  }
                  else
                  {
                    /* Do Search-Replace */
                    for (templ=rangestart;templ!=rangeend;templ++)
                    {
                      tempi = replacestronce(cline,nodeptr->flline,aline,bline,256,0);
LoopReplace: /* Another small bodge */
                      if (tempi==-2)
                      {
                        printf("Replace makes line too long!\n");
                      }
                      else
                      {
                        if (tempi>=0)
                        {
                          if (searchcheck==1)
                          {
                            printf("Replaced line:\n  %ld\t",templ);
                            if (curline==templ) printf("*"); else printf(" ");
                            printf(":%s",cline);
                            if (instrch(cline,10,0)==-1) printf("\n-");
                            if (yesnomsg(repcorrectmsg)!=0)
                            {
                              strcpy(nodeptr->flline,cline);
                            }
                            else
                            {
                              /* This works and seems OK, but may need checking as I'm not sure about it */
                              tempi-=strlen(bline);
                              tempi+=strlen(aline);
                            }
                          }
                          else
                          {
                            printf("  %ld\t",templ);
                            if (curline==templ) printf("*"); else printf(" ");
                            printf(":%s",cline);
                            if (instrch(cline,10,0)==-1) printf("\n-");
                            strcpy(nodeptr->flline,cline);
                          }
                          
                          tempi = replacestronce(cline,nodeptr->flline,aline,bline,256,tempi);
                          goto LoopReplace;
                        }
                      }
                      
                      if (nodeptr->next == 0) break;
                      nodeptr = nodeptr->next;
                    }
                  }
                }
              }
              else printf("Cancelled!");
            }
          break;
          
          case (64+20):
          case (96+20):
            /* Transfer (merge) */
            rangestart = getfredlinlnnum(acmd,0);
            switch (rangestart)
            {
              case FLN_CURRENT:
              case FLN_NONUMBER:
                rangestart = curline;
              break;
              
              case FLN_INVALIDNUMBER:
              case 0:
                /*printf("Entry Error!\n");*/
                printf(enterr);
              break;
            }
            strcpy(aline, acmd+firstletterpos(acmd)+1);
            tempi=instrch(aline,10,0);
            if (tempi>=0) aline[tempi]=0;
            strltrim(bline, aline);
            strcpy(aline,bline);
            bline[0]=0;
            if ((rangestart !=FLN_INVALIDNUMBER) && (rangestart != 0))
            {
              /* Find the right node */
              nodeptr = root;
              templ=0;
              rangestart--;
              while (templ!=rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              tnodeptr=nodeptr->next;
              
              if ((aline[0]==0) || (access(aline,F_OK) == -1))
              {
                printf("File not found.\n");
              }
              else
              {
                if (!(fp = fopen(aline,"r")))
                {
                   printf("Error opening file!\n");
                }
                else
                {
                  /* Read file */
                  templ=0;
                  while ( fgets(readstring,256,fp) != NULL )
                  {
                    insnodeptr = (struct flnode *) malloc( sizeof(struct flnode) );
                    if (insnodeptr == 0)
                    {
                      printf("Memory full!\n");
                      break;
                    }
                    /* Load in text */
                    strcpy(insnodeptr->flline,readstring);
                    insnodeptr->next = 0;
                    /* Set up next */
                    nodeptr->next = insnodeptr;
                    nodeptr=insnodeptr;
                    insnodeptr=insnodeptr->next;
                    templ++;
                  }
                  if (ferror(fp)) printf("Error reading file!\n");
                  fclose(fp);
                  nodeptr->next = tnodeptr;
                  
                  printf("%ld lines loaded into document.\n",templ);
                }
              }
            }
          break;
          
          case (64+13):
          case (96+13):
            /* Move */
            if (firstletterpos(acmd)==0)
            {
              rangestart=FLN_NONUMBER;
              rangeend=FLN_NONUMBER;
              moveline=FLN_NONUMBER;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if (rangestart == FLN_CURRENT) rangestart = curline;
              if (rangestart == FLN_NONUMBER) rangestart = 1;
              tempi = instrch(acmd,'-',0); 
              if (tempi==-1) tempi = instrch(acmd,',',0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
                moveline = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
                templ = instrch(acmd,',',tempi+1);
                if (templ==-1) tempi=-1;
                else tempi += instrch(acmd,',',tempi+1) + 1;
                if (tempi==-1)
                {
                  if (instrch(acmd,'-',0)==-1)
                  {
                    moveline = rangeend;
                    rangeend = FLN_NONUMBER;
                  }
                  else
                  {
                    moveline = FLN_NONUMBER;
                  }
                }
                else
                {
                  moveline = getfredlinlnnum(acmd,tempi+1);
                }
              }
              if (rangeend==FLN_CURRENT) rangeend = curline;
              if (rangeend==FLN_NONUMBER) rangeend = rangestart;
              if (moveline==FLN_CURRENT) moveline = curline;
            }
            if ((rangestart<1) || ((rangeend!=FLN_AFTERLAST) && 
                (rangeend<rangestart)) || (moveline==FLN_NONUMBER) || 
                (moveline==0) || (moveline==FLN_INVALIDNUMBER))
            {
              printf(enterr);
            }
            else
            {
              if (((rangeend==FLN_AFTERLAST) && (moveline>=rangestart)) || 
                  ((moveline>=rangestart) && (moveline<=rangeend)) || 
                  (moveline==rangeend))
              {
                printf("Cannot move range inside itself!\n");
              }
              else
              {
                /* Find the right node */
                nodeptr = root;
                templ=0;
                nlines = 0;
                rangestart--;
                while (templ<rangestart)
                {
                  if (nodeptr->next == 0) break;
                  nodeptr = nodeptr->next;
                  templ++;
                }
                if (templ<rangestart)
                {
                  printf(enterr);
                }
                else
                {
                  tnodeptr=nodeptr;
                  while (templ!=rangeend)
                  {
                    if (nodeptr->next == 0) break;
                    nodeptr = nodeptr->next;
                    nlines++;
                    templ++;
                  }
                  insnodeptr=nodeptr;
                  nodeptr = root;
                  templ=0;
                  moveline--;
                  while (templ!=moveline)
                  {
                    if (nodeptr->next == 0) break;
                    nodeptr = nodeptr->next;
                    templ++;
                  }
                  if (templ<moveline)
                  {
                    printf(enterr);
                  }
                  else
                  {
                    /* Move Nodes */
                    movenodeptr = nodeptr;
                    nodeptr = movenodeptr->next;
                    movenodeptr->next = tnodeptr->next;
                    tnodeptr->next = insnodeptr->next;
                    insnodeptr->next = nodeptr;
                    printf("%ld lines moved to %ld.\n",nlines,moveline+1);
                  }
                }
                
              }
            }
          break;
          
          case (64+3):
          case (96+3):
            /* Copy lines */
            if (firstletterpos(acmd)==0)
            {
              rangestart=FLN_NONUMBER;
              rangeend=FLN_NONUMBER;
              moveline=FLN_NONUMBER;
              copytimes=FLN_NONUMBER;
            }
            else
            {
              rangestart = getfredlinlnnum(acmd,0);
              if (rangestart == FLN_CURRENT) rangestart = curline;
              if (rangestart == FLN_NONUMBER) rangestart = 1;
              tempi = instrch(acmd,45,0);
              if (tempi==-1) tempi = instrch(acmd,44,0);
              if (tempi==-1)
              {
                rangeend = FLN_NONUMBER;
                moveline = FLN_NONUMBER;
                copytimes = FLN_NONUMBER;
              }
              else
              {
                rangeend = getfredlinlnnum(acmd,tempi+1);
                templ = instrch(acmd,44,tempi+1);
                if (templ==-1) tempi=-1; else
                tempi += instrch(acmd,44,tempi+1) + 1;
                if (tempi==-1)
                {
                  copytimes = FLN_NONUMBER;
                  if (instrch(acmd,45,0)==-1)
                  {
                    moveline = rangeend;
                    rangeend = FLN_NONUMBER;
                  }
                  else
                  {
                    moveline = FLN_NONUMBER;
                  }
                }
                else
                {
                  moveline = getfredlinlnnum(acmd,tempi+1);
                  templ = instrch(acmd,44,tempi+1);
                  if (templ==-1) tempi=-1; else
                  tempi += instrch(acmd,44,tempi+1) + 1;
                  if (tempi==-1)
                  {
                    copytimes = FLN_NONUMBER;
                  }
                  else
                  {
                    copytimes = getfredlinlnnum(acmd,tempi+1);
                  }
                }
              }
            }
            if (rangeend==FLN_CURRENT) rangeend = curline;
            if (rangeend==FLN_NONUMBER) rangeend = rangestart;
            if (moveline==FLN_CURRENT) moveline = curline;
            if (copytimes==FLN_NONUMBER) copytimes = 1;
            if ((rangestart<1) || ((rangeend!=FLN_AFTERLAST) && 
                (rangeend<rangestart)) || (moveline==FLN_NONUMBER) || 
                (moveline==0) || (moveline==FLN_INVALIDNUMBER) || 
                (copytimes<1))
            {
              printf(enterr);
            }
            else
            {
              nodeptr = root;
              templ=0;
              while (templ<rangestart)
              {
                if (nodeptr->next == 0) break;
                nodeptr = nodeptr->next;
                templ++;
              }
              if (templ<rangestart)
              {
                printf(enterr);
                break;
              }
              else
              {
                tnodeptr = nodeptr;
                ttnodeptr = tnodeptr;
                nodeptr = root;
                templ=0;
                moveline--;
                while (templ!=moveline)
                {
                  if (nodeptr->next == 0) break;
                  nodeptr = nodeptr->next;
                  templ++;
                }
                movenodeptr = nodeptr;
                
                copyroot = (struct flnode *) malloc( sizeof(struct flnode) ); /* Build the node */
                if (copyroot == 0)
                {
                  printf("Out of memory!\n");
                }
                else
                {
                  copyroot->next = NULL; /* Only one in list, so point to NULL */
                  clearstring(copyroot->flline, 256);
                }
                while ((copyroot != 0) && (copytimes>0))
                {
                  insnodeptr = copyroot;
                  for (templ=rangestart;templ!=rangeend+1;templ++)
                  {
                    if (tnodeptr == 0) break;
                    insnodeptr->next = (struct flnode *) malloc( sizeof(struct flnode) );
                    if (insnodeptr->next == 0)
                    {
                      printf("Memory full!\n");
                      copytimes = 0;
                      break;
                    }
                    insnodeptr = insnodeptr->next;
                    strcpy(insnodeptr->flline,tnodeptr->flline);
                    tnodeptr = tnodeptr->next;
                  }
                  insnodeptr->next = movenodeptr->next;
                  movenodeptr->next = copyroot->next;
                  copytimes--;
                  tnodeptr = ttnodeptr;
                }
                if (copyroot != 0) free(copyroot);
              }
            }
          break;
          
          case (64+8):
          case (96+8):
#ifndef NOHELP
            /* Help */
            strcpy(aline, acmd+firstletterpos(acmd)+1);
            strltrim(bline, aline);
            strcpy(aline, bline);
            switch (aline[0])
            {
              case 0:
              case 13:
              case 10:
              case '0':
                /* Index */
                printf("Help Menu:\n");
                printf("Enter \"H\" followed by the letter relating to the topic you wish to read.\n");
                printf("A	About FREDLIN\n");
                printf(".	The line numbering system\n");
                printf("-	Typing in FREDLIN\n");
                printf("1	Editing lines\n");
                printf("L	Listing lines\n"); /* P will also be covered */
                printf("Q	Quitting FREDLIN\n"); /* E will also be covered */
                printf("W	Saving your file\n");
                printf("I	Inserting lines\n");
                printf("D	Deleting lines\n");
                printf("M	Moving lines\n");
                printf("C	Copying lines\n");
                printf("S	The \"Search\" command\n");
                printf("R	Replacing text\n");
                printf("T	Merging files\n");
                printf("V	Version information and Credits\n");
              break;
              
              case 'A':
              case 'a':
                /* About */
                printf("About FREDLIN:\n");
                printf("The idea for FREDLIN arose in 2010 with my desire for \"ed\" to be easier to \nuse.  It was my first time editing in Linux with the command line and only \n\"ed\" and \"vi\" were available.  I have never been able to get on with \n\"vi\", so I was left struggling with \"ed\".\n\n");
                printf("The DOS line editor \"EDLIN\" was created by Tim Paterson (the genius behind \nthe original release of DOS) in about 1980.  It has long been a simple-to-use \nline editor and I thus quickly identified it as a good replacement, at least \nfor what I was doing.  I searched long and hard online to try to see if there \nwas any project to port it across, but in vain.\n\n");
                printf("Years later, I decided to port it across.  I thought I might be onto \nsomething when I found the source online for the FreeDOS version, however the \nauthor had changed too much of the program for my taste.  I even looked \nat disassembling the original program, but didn't get very far.  In the end, I \ndecided to re-write it from scratch.\n\n");
                keypause();
                printf("I had to change a few Ctrl actions for compatibility with the Linux \nplatform and write a new line-entry system to support the original 86-DOS \n\"Template\" system, but the result you see before you: the result of five \nyears of procrastination, plus a couple of week's work.\n\n");
                printf("Please note that this is the first program I've written in C in four years, so \napologies if it is not as efficient as it could be.\n");
              break;
              
              case '.':
              case '>':
                /* Line numbers */
                printf("The line numbering system:\n");
                printf("Line numbers in FREDLIN are dynamically assigned to the lines in the file and \nwill change if a new line is inserted (see Inserting lines [HI]) earlier in \nthe file.  A specific line (initially the first, then usually the last to be \nedited) is designated as the \"current\" line and is marked by an asterisk \n(\"*\") when the file is listed (see Listing lines [HL]).\n\n");
                printf("A line number can be specified in the following ways:\n");
                printf("-	As an actual number (the number of the line) between 1 and \n\t2147483647.  If the number is greater than the largest existing line \n\tnumber then the line after the last existing line will be specified.\n");
                printf("-	As a full stop (\".\"), which denotes the \"current\" line.\n");
                printf("-	As a hash (\"#\"), which denotes the line after the last \n\tone currently in existence (this is the same as specifying a line \n\tnumber greater than that of the last line in the file).\n");
                printf("\nIf no line number is given when one is required by a command, it will \ndefault to a certain value (dependent upon the command in question).\n\n");
                printf("A range of lines can be specified two line numbers separated either by a \ncomma (\",\") or a dash (\"-\") but NOT a space.  A range CANNOT be written \nwithout an explicit end (\"3-\"): to specify lines 3 to the end use \"3-#\".\n");
              break;
              
              case '-':
              case '_':
                /* Typing */
                printf("Typing in FREDLIN:\n");
                printf("FREDLIN uses a \"Template\" system for commands.  When typing a command, the \ntemplate is the last command typed.  When editing a line (see Editing lines \n[H1]), the template is the original contents of the line.  When inserting \nlines (see Inserting lines [HI]), the template is the last line to be \ninserted.  When typing replacement text for a \"Replace\" command (see \nReplacing text [HR]), the template is the text to find.  As you type, the \ncurrent position in the template is advanced unless \"Insert Mode\" is on.\n\n");
                printf("The following key combinations work with the template:\n");
                printf("ESC S or F1	This copies one character from the template to the current line.\n");
                printf("ESC T or F2	This must be followed by a character and copies all characters \n\t\tfrom the template up to but not including the next occurrence \n\t\tin the template of the specified character.  If the specified \n\t\tcharacter does not occur from the current point in the \n\t\ttemplate onwards, no characters are copied to the line and the \n\t\tposition in the template does not advance.\n");
                printf("ESC U or F3	This copies all remaining characters from the template to \n\t\tthe line.\n");
                printf("ESC V or F4	This skips over one character in the template.\n");
                keypause();
                printf("ESC W or F5	This must be followed by a character.  It skips over all \n\t\tcharacters in the template, up to but not including the next \n\t\toccurrence of the specified character in the template after \n\t\tthe current point.  If the specified character does not \n\t\tappear, no characters are skipped.\n");
                printf("ESC P		This turns on \"Insert Mode\".  In this mode, as characters are \n\t\ttyped to the line, the current position in the template does \n\t\tnot advance.\n");
                printf("ESC Q		This turns off \"Insert Mode\".  Outside of this mode, the \n\t\tposition in the template is advanced for each character typed \n\t\tto the line.  This is the default mode.\n");
                printf("INS		This toggles \"Insert Mode\" on and off.  By default, it is \n\t\toff.\n");
                printf("ESC R or F8	This changes the template to be the current line.  The current \n\t\tline as typed into the buffer is cleared and \"Insert Mode\" is \n\t\tturned off.  On the screen, an At sign (\"@\") is written \n\t\tfollowed by a new line.\n");
                keypause();
                printf("\nA line ends with a new line, the new line character being included in the \nline.  The maximum length of a line is 255 characters, however you can make a \nline \"wrap\" to a new line.  To do this, end the line with a CTRL+Y.  This \nsubmits the line without the new line character appended to the end.  If \nediting a line in this way you will need to insert (see the \"I\" command) the \nremains of the line afterwards.\n\n");
                printf("You can clear the current line and start again by pressing CTRL+X.\n\n");
#ifdef SIGINTTOCANCEL
                printf("You can cancel the current line by pressing CTRL+C.  This will cancel editing \nand leave from inserting text, discarding the current line.  Any lines entered \npreviously will not be affected.  All commands can be terminated in this way \nunless they require a specific Y/N answer.\n\n");
#else
                printf("You can cancel the current line by pressing CTRL+D.  This will cancel editing \nand leave from inserting text, discarding the current line.  Any lines entered \npreviously will not be affected.  All commands can be terminated in this way \nunless they require a specific Y/N answer.\n\n");
#endif
                printf("CTRL+L is used to specify the use of a line break in search or replace text as \npart of the Search command (see The \"Search\" command [HS]) or Replace \ncommand (see Replacing text [HR]) respectively.  It is therefore recommended \nthat you do not use this character as part of the text in your file.\n");
              break;
              
              case '1':
              case '!':
                /* Editing lines */
                printf("Editing Lines:\n");
                printf("To edit a line, simply type its line number as a command.  If the line number \nis not specified (that is, a blank command is entered) the line after the \n\"current line\" (see The line numbering system [H.]) is edited.  You can only \nedit lines that already exist: to enter text in an empty file, you will need \nto use the Insert command (see Inserting lines [HI].\n\n");
#ifdef SIGINTTOCANCEL
                printf("The line to be edited will be displayed with its line number and the new line \nwill be entered below it.  If no changes are needed, press CTRL+C to cancel \nthe edit: do not press Return as this will replace the line with a blank \nline.  Otherwise, you can enter your replacement line with the old one as the \n\"Template\" (see Typing in FREDLIN [H-]).\n");
#else
                printf("The line to be edited will be displayed with its line number and the new line \nwill be entered below it.  If no changes are needed, press CTRL+D to cancel \nthe edit: do not press Return as this will replace the line with a blank \nline.  Otherwise, you can enter your replacement line with the old one as the \n\"Template\" (see Typing in FREDLIN [H-]).\n");
#endif
              break;
              
              case 'l':
              case 'L':
              case 'p':
              case 'P':
                /* Listing lines */
                printf("Listing lines:\n");
                printf("There are two ways of listing lines in FREDLIN: the List command and the Page \ncommand.  Both list a specified range of lines.  The lines are listed preceded \nby the line number (see The line numbering system [H.]) and an asterisk \n(\"*\") if it is the current line.  If a line does not end with a line-break \nand thus \"wraps\" to the next line, the following line (the one it wraps to) \nwill be preceded by a hyphen (\"-\").  A file that does not end with a linebreak\nwill, when listed, display a hyphen before the next prompt.\n\n");
                printf("The List command:\n");
                printf("The list command comprises of a range of lines (see The line numbering system \n[H.]) followed by the letter \"L\".  If the first line number in the range is \nnot specified, the \"current line\" (see The line numbering system [H.]) will \nbe assumed.  If the second line number in the range is omitted, 23 lines will \nbe listed; from 11 lines before the first line number to 11 after it.  If the \nfirst line is fewer than 11 lines before the first line number then additional \nlines will be listed at the end.  Therefore, if the range is omitted entirely \n(and thus the command consists entirely of the letter \"L\"), 23 lines, \nstarting from 11 before the current line if possible, will be listed.  The \n\"current line\" is not changed.\n\n");
                keypause();
                printf("The Page command:\n");
                printf("The page command also comprised of a range of lines (see The line numbering \nsystem [H.]) but followed by the letter \"P\".  If the first line number in \nthe range is not specified then the first line in the file is assumed.  If the \nsecond line number in the range is omitted then the last line in the file is \nassumed.  Therefore, if the range is not specified, the whole file will be \nlisted.  The \"current line\" is then changed to be the last line listed.\n");
              break;
              
              case 'q':
              case 'Q':
              case 'e':
              case 'E':
                /* Quitting FREDLIN */
                printf("Quitting FREDLIN:\n");
                printf("There are two ways to quit FREDLIN: the Quit command and the Exit command.\n\n");
                printf("The Quit command:\n");
                printf("The Quit command comprises simply of the letter \"Q\".  It quits FREDLIN \nwithout saving the open file, thus if the file has changed since it was last \nsaved then any changes are lost.  If changes to the file have been saved since \nthe file was opened, the original file will be preserved as a file with \n\".BAK\" appended to the filename.  FREDLIN will ask you to confirm that you \nwish to quit before taking this action.\n\n");
                printf("The Exit command:\n");
                printf("The Exit command comprises simply of the letter \"E\".  It saves any changes \nin the open file first before quitting FREDLIN.  The original file (before \nediting) is preserved with \".BAK\" appended to its filename.  FREDLIN will not \nask you to confirm before taking this action.\n");
              break;
              
              case 'w':
              case 'W':
                /* Saving your file */
                printf("Saving your file:\n");
                printf("To save your file, use the Write command.  This comprises simply of the letter \n\"W\".  The file as it exists in memory will be saved to the original \nfilename.  The original file will be preserved, however, with \".BAK\" \nappended to its filename.\n");
              break;
              
              case 'i':
              case 'I':
                /* Inserting lines */
                printf("Inserting lines:\n");
                printf("To insert lines into the file, use the Insert command.  This is comprised of a \nline number followed by the letter \"I\".  The new line will be inserted \nbefore the specified line.  If the line number is not specified, the \"current \nline\" (see The line numbering system [H.]) is assumed.  If a line number \ngreater than the number of lines in the file is specified then the new line \nwill be appended to the end of the file.\n\n");
#ifdef SIGINTTOCANCEL
                printf("After entering this command, you can enter as many lines as you wish, ending \nthem with a return (or a CTRL+Y if you wish the text to \"wrap\" onto the next \nline).  To exit from this mode back to entering commands, press CTRL+C.  When \nyou do this, the line you were entering at the time will be discarded but any \nlines entered prior to this will be inserted into the file at the specified \npoint.  The last line inserted will become the \"current line\" (see The line \nnumbering system [H.]).\n");
#else
                printf("After entering this command, you can enter as many lines as you wish, ending \nthem with a return (or a CTRL+Y if you wish the text to \"wrap\" onto the next \nline).  To exit from this mode back to entering commands, press CTRL+D.  When \nyou do this, the line you were entering at the time will be discarded but any \nlines entered prior to this will be inserted into the file at the specified \npoint.  The last line inserted will become the \"current line\" (see The line \nnumbering system [H.]).\n");
#endif
              break;
              
              case 'd':
              case 'D':
                /* Deleting lines */
                printf("Deleting lines:\n");
                printf("To delete lines in a file, use the Delete command.  This is comprised of a \nrange followed by the letter \"D\".  Please note that ALL the lines in the \nspecified range will be deleted.  If the first line number in the range is \nomitted, the \"current line\" (see The line numbering system [H.]) is \nassumed.  If the second line number in the range is omitted, it is assumed to \nbe the same as the first so that only one line is deleted.  Therefore, if the \nrange is omitted, the current line is deleted.  The line immediately following \nthe deleted text will then become the \"current line\" (or the last line if \nthe range was at the end of the file) and will have the same line number as \nthe first line deleted.  FREDLIN will then tell you how many lines were \ndeleted.\n");
              break;
              
              case 'm':
              case 'M':
                /* Moving lines */
                printf("Moving lines:\n");
                printf("To move lines to another part of the file, use the Move command.  This is \ncomprised of a range representing the lines you want to move, followed by a \ncomma (\",\"), followed by the line number to which you want to move these \nlines, followed by the letter \"M\".  The range of lines will be moved to the \nposition between the specified line and the one before it.  If the first line \nnumber in the range is omitted, the first line in the file will be assumed.  \nIf the second line number in the range is omitted, it is assumed to be the \nsame as the first; that is, only one line will be moved.  If the destination \nline number is omitted, the command will fail.  If the destination line is \nwithin the range of lines to move, the command will fail.\n");
              break;
              
              case 'c':
              case 'C':
                /* Copying lines */
                printf("Copying lines:\n");
                printf("To copy lines to another part of the file, use the Copy command.  This is \ncomprised of the range to copy, followed by a comma (\",\"), followed by the \nline to which to copy the lines, followed by another comma, followed by the \nnumber of copies to make, followed by the letter \"C\".  The range of lines \nwill be copied to the position between the specified line and the line before \nit.  If the first line number in the range is omitted, the first line in the \nfile is assumed.  If the second line in the range is omitted, it is assumed to \nbe the same as the first (that is, only one line will be copied).  If the \ndestination line is omitted, the command will fail.  If the number of copies \nto make is omitted, it is assumed to be 1 (that is, only one copy will be \nmade).\n");
              break;
              
              case 's':
              case 'S':
                /* The "Search" command */
                printf("The \"Search\" command:\n");
                printf("To search for a specific piece of text in the file, use the \"Search\" \ncommand.  You can only search for a piece of text that exists entirely on one \nline: if a line ends or \"wraps\" during that text then it will not be found.  \nThe Search command is comprised of a range (indicating where is to be \nsearched), followed by an optional question mark (\"?\"), followed by the \nletter \"S\", followed by the text for which to search.  If the first line \nnumber in the range is omitted, the first line in the file is assumed.  If the \nsecond line number in the range is omitted, the last line in the file is \nassumed.  Therefore, if the range is omitted, the whole file is searched.  If \nthe text for which to search is omitted, the command will fail.\n\n");
                printf("The line break at the end of the command is not considered part of the text \nfor which to search.  As such, if you wish to search for text at the end of \nthe line (that is, that includes the line break), you can substitute in the \nCTRL+L key combination (which will appear as a colour-inverted letter \"L\" on \nscreen, assuming your display allows for colour inversion).  It is as such \nrecommended that you do not include this key combination as part of the file \nitself.\n\n");
                keypause();
                printf("The specified range of lines will be searched for the specified search text.  \nIf the search text is not found in any of the specified lines, FREDLIN will \ninform you of this, otherwise the line containing the found text will be \nlisted (see Listing lines [HL] for the format).  If the question mark (\"?\") \nhas been specified before the \"S\" in the command then you will be asked \nwhether or not this is the correct occurence of the search text in the range.  \nIf the answer to this question is \"Y\" or if the question mark is not \nspecified in the command then the line of the found search text becomes the \ncurrent line and the command will end.  Otherwise, the search will continue \nand you will be prompted each time the search text is found.\n");
              break;
              
              case 'r':
              case 'R':
                /* Replacing text */
                printf("Replacing text:\n");
                printf("You can search for a specific piece of text in the file and have it \nautomatically replaced with another piece of text using the Replace command.  \nThis is comprised of a range specifying where to search, followed by an \noptional question mark (\"?\"), followed by the letter \"R\", followed by the \ntext for which to search.  After entering this command, you will be prompted \nto enter the text with which to replace the search text.  You can then enter \nit, using the search text as the \"Template\" (see Typing in FREDLIN [H-]).  \nIf the first line number in the range is omitted, the first line in the file \nis assumed.  If the second line number in the range is omitted, the last line \nin the file is assumed.  Therefore, if the range is omitted, the entire file \nwill be searched.  If the search text is omitted, the command will fail.  If \nthe replace text is omitted (that is, a blank line is submitted) then it is \nconsidered to be \"null\", effectively deleting occurences of the search text.\n\n");
                keypause();
                printf("If the question mark (\"?\") is not specified before the letter \"R\" in the \ncommand then all occurences of the search string in the specified lines will \nautomatically be replaced by the replace text and the line in which the \nreplacement occurs displayed.  Otherwise, every time the search text is found \nin the specified lines, FREDLIN will display the line as it would be if the \nreplacement was made.  You will then be asked if this is correct.  If you \nanswer \"Y\", the replacement will be made, otherwise it will not.  The search \nwill then continue, prompting you for each replacement, until all the \nspecified lines have been searched.\n\n");
                printf("The line break at the end of the command is not considered part of the text \nfor which to search or the replacement text.  As such, if you wish to search \nfor text at the end of the line (that is, that includes the line break), you \ncan substitute in the CTRL+L key combination (which will appear as a \ncolour-inverted letter \"L\" on screen, assuming your display allows for \ncolour inversion).  You should then use it in the correct place in the \nreplacement string unless you want the replacement lines to \"wrap\".  Do not \nuse it to place line-breaks in the middle of lines as this will cause \nunpredictable behaviour and you may lose the end of the line.  It is as such \nrecommended that you do not include this key combination as part of the file \nitself.\n\n");
                keypause();
#ifdef SIGINTTOCANCEL
                printf("The CTRL+C command cannot be used to stop the Replace command once it has \nbegun, so it is recommended that you are very careful with this command.\n");
#else
                printf("The CTRL+D command cannot be used to stop the Replace command once it has \nbegun, so it is recommended that you are very careful with this command.\n");
#endif
              break;
              
              case 'T':
              case 't':
                /* Merging files */
                printf("Merging files:\n");
                printf("FREDLIN allows you to merge another file into the one that is currently open \nby using the Transfer command.  This comprises of the line number of the line \nat which to insert the file, followed by the letter \"T\", followed by the \nfilename (including the path if it's in another directory) of the file you \nwish to merge into the current file.  The specified file will be inserted \nbetween the specified line and the line before it, such that the first line of \nthe inserted file becomes the specified line.  If the line number is not \nspecified, the \"current line\" (see The line numbering system [H.]) will be \nassumed.\n");
              break;
              
              case 'V':
              case 'v':
                /* Version information and Credits */
                printf("Version information and Credits:\n");
                printf("FREDLIN %s\n",progver);
                printf("%s\nContact me on Twitter @DHeadshot or on the Fediverse @dheadshot@mastodon.social\n", progcopyright);
                printf("This program is now Free Open-Source Software  - distribute it as you like.\n\n");
                printf("This program uses the Quick and Dirty Input Library version %s.  See\nhttp://github.com/dheadshot/libqdinp2 for more details.\n\n", qdinpver());
                printf("The \"mygetch\" routine used as a basis for the input library was created by VvV\nand kermi3 on the CProgramming.com boards, so thanks to them for that.\n\n");
                printf("FREDLIN was inspired by the original EDLIN, created by Tim Paterson.  None of \nhis code or that of Microsoft has been used in this program though.\n");
              break;
            }
#else
            printf("This version of FREDLIN has been compiled without the built-in \"Help\" file.\nPlease refer to the file \"%s\" that came with this program.\n", proghelpfile);
#endif
          break;
          
          
          default:
            printf("%s",enterr);
          break;
        }
      }
      /*printdebug(acmd);*/
    }
    else if (ret == 3||ret == 4)
    {
      printf("^%c\n",ret+64);
    }
  }
  
  /*--------------------------------------------------------*/
  
  /* clean up */
  nodeptr = root->next;
  while (nodeptr != 0)
  {
    tnodeptr = nodeptr->next;
    free(nodeptr);
    nodeptr = tnodeptr;
  }
  free(root);
  
  /* Exit */
  return 0;
}
