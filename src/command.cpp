#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dd.h"

#include "config.h"
#include "command.h"
#include "message.h"

#include <strings.h>
#define stricmp strcasecmp

//----------------------------------------------------
//defs for system options

//forces a res on startup
int option_res(char *p)
{
 int xw,yw;
 if (sscanf(p,"%d %d",&xw,&yw)<2) return 0;
 SCREENX=xw;
 SCREENY=yw;
 return 1;
}

int option_help(char *p);
int option_sb(char *p);
int option_sb16(char *p);

static char *inputtypes[]={"NONE","GRAVIS","KEY1","KEY2","JOY1","JOY2",0};
int getinputtype(char *s)
{
 for (int i=0; inputtypes[i]; i++)
  if (!stricmp(s,inputtypes[i])) return i;

 msg.error("Invalid input type %s. Valid types are: ",s);
 for (int i=0; inputtypes[i]; i++) msg.error(inputtypes[i]);
 return 0;
}

int option_setinput(char *p)
{
 int num;
 char type[32]; type[0]=0;
 if (!sscanf(p,"%d %s",&num,type)) return 0;
 num--;
 if (num<0 || num>3) {msg.error("Input num out of range"); return 1;}

 cfg->pinput[num]=getinputtype(type); //set input
 return 1;
}

optiondef optiondefs[]=
{
 #ifdef DOS
 {"?",option_help,0, 0,"Display this help"},
 {"h",option_help,0, 0, 0},
 #endif

 {"res",     option_res,0,    "<X-res> <Y-res>", "Change video resolution to (X-res,Y-res)"},
 {"nosound", 0,1,             0,"Disable sound"},

 #ifdef DOS
 {"sb"     , option_sb,0,   "<port> <irq> <dma>", "Manually set SB hardware settings"},
 {"sb16"   , option_sb16,0, "<port> <irq> <dma>", "Manually set SB16 hardware settings"},

 {"novesa" , 0,0,0, "Do not use VESA 2.0 extensions"},
 {"banked" , 0,0,0, "Force banked video mode"},
 {"linear" , 0,0,0, "Force linear video mode"},
 #endif

 {"setinput",option_setinput,0, "<num> <type>","Set input device <num> to <type>"},

// {"definekey",option_definekey,0, "<num> <scancodes...>","Redefine keys for keyboard <num> "},

 {0,0,0,0,0}
};


//------------------------------------------------------------
//defs for commands to be executed after system initialization

void disablegui();
void m_showmessages();
void m_showfps();
int cmd_message(char *p) {m_showmessages(); return 1;}
int cmd_showfps(char *p) {m_showfps(); return 1;}
int cmd_hidegui(char *p) {disablegui(); return 1;}
int cmd_maximize(char *p);
int cmd_loadrom(char *p);
int cmd_runrom(char *p);


int cmd_server(char *p);
int cmd_connect(char *p) {return 1;}

optiondef commanddefs[]=
{
 {"message", cmd_message, 0, 0, "Display message box"},
 {"showfps", cmd_showfps, 0, 0, "Show frames/sec"},
 {"hidegui", cmd_hidegui, 0, 0, "Hide GUI on startup"},
 {"load",    cmd_loadrom, 0, "<filename>", "Load ROM <filename>"},
 {"run",    cmd_runrom, 0, "<filename>", "Load & run ROM <filename>"},
 {0,0,0,0,0}
};

//----------------

//find optiondef with specific name, 0 if not found
optiondef *findoptiondef(char *name,optiondef *od)
{
 optiondef *c=od;
 for ( ; c->name; c++)
  if (!stricmp(c->name,name)) return c;
 return 0;
}

int getoption(char *name)
{
 optiondef *od=findoptiondef(name,optiondefs);
 return od ? od->flag : 0;
}

int getcommand(char *name)
{
 optiondef *od=findoptiondef(name,commanddefs);
 return od ? od->flag : 0;
}


void optiondef::printhelp()
{
 if (!desc) return;
 char s[128];
 sprintf(s," -%s %s",name,syntax ? syntax : "");
 memset(s+strlen(s),' ',32);
 s[26]=0;
 strcat(s,desc);
 printf("%s\n",s);
}

//displays help
int option_help(char *p)
{
 optiondef *c;

 #ifdef DOS
 printf("System options: \n");
 for (c=optiondefs; c->name; c++)  c->printhelp();
 printf("\n");
 printf("Commands: \n");
 for (c=commanddefs; c->name; c++)  c->printhelp();
 exit(1);
 #endif
 return 1;
}



//----------------------------------------
//individual option on option line

void option::free()
{
 if (name) ::free(name);
 if (parm) ::free(parm);
}

void option::print()
{
 if (!name) return;
 if (parm) msg.printf(1,"%s: %s",name,parm);
      else msg.printf(1,name);
}

//execute a particular commandline option by finding it's
//option def, setting the flag and calling the func
void option::execute(int type)
{
 if (!name) return;

 optiondef *od=findoptiondef(name,type ? commanddefs : optiondefs);
 if (!od)
  {
   if (!type && !findoptiondef(name,commanddefs))
         msg.error("unrecognized arg: %s",name);
   return; //no option
  }

 od->flag=1;
 if (od->func && !od->func(parm ? parm : "")) //error executing option....
   msg.error("syntax error: %s %s",od->name,od->syntax);
}


//----------------------------------------
//class for parsing/executing commandline

//see if something is a option
inline int isoption(char *s) {return s[0]=='-';}
inline int isfile(char *s) {return s[0]=='@';}
inline int isparm(char *s) {return s[0]!='-' && s[0]!='@';}

//add option to end of array
option *commandline::addoption()
{
 options=(option *)realloc(options,(numoptions+1)*sizeof(option));
 options[numoptions].clear();
 return &options[numoptions++];
}

//print all commands in commandline
void commandline::print()
{
 msg.printf(2,"%d options parsed:",numoptions);
 for (int i=0; i<numoptions; i++)
  options[i].print();
}

//execute all commands on commandline
void commandline::execute(int type)
{
 for (int i=0; i<numoptions; i++)  options[i].execute(type);
}


//parse commands from array of strings
void commandline::parse(int argc,char **argv)
{
 int i=0;
 while (i<argc)
  if (isoption(argv[i])) //see if it's a option
   {
    option *C=addoption(); //add new option...
    C->name=strdup(argv[i]+1); //add option itself

    char p[128]; p[0]=0; //string containing parms for option
    for (i++; i<argc && isparm(argv[i]); i++)
          {if (p[0]) strcat(p," "); strcat(p,argv[i]);}
    C->parm=strdup(p); //add parms
   } else
   if (isfile(argv[i])) //see if it's a file redirection
   {
    char fname[64];
    strcpy(fname,argv[i++]+1);
    if (!strchr(fname,'.')) strcat(fname,".cfg");

    msg.printf(1,"Parsing file %s",fname);
    FILE *f=fopen(fname,"rt"); //open file
    if (f) parse(f); //parse filename
    fclose(f);
   } else
   msg.error("misplaced argument: %s",argv[i++]); //it's misplaced
}


//parse commands from a string
void commandline::parse(char *c)
{
 int argc=0;
 static char *argv[32];

 while (*c) //until end of commandline...
 {
  while (*c && isspace(*c)) c++; //scan through all spaces
  if (!*c) break;
  argv[argc++]=c; //store string start
  while(*c && !isspace(*c)) c++; //scan through all non-spaces
  if (!*c) break;
  *c=0; //put end of string
  c++;
 }
 parse(argc,argv);
}

//parse commands from a file
void commandline::parse(FILE *f)
{
 char line[256];
 while (fgets(line,256,f)) parse(line);
}

//constructors....
commandline::commandline(int argc,char **argv) {numoptions=0; options=0;parse(argc,argv);}
commandline::commandline(char *c) {numoptions=0; options=0; parse(c);}
commandline::commandline(FILE *f) {numoptions=0; options=0; parse(f);}

//destructor
commandline::~commandline()
{
 for (int i=0; i<numoptions; i++)  options[i].free();
 free(options);
}
