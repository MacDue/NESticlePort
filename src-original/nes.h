#ifndef _NES_
#define _NES_

struct NESROMHEADER
{
 char str[4]; //"NES" string
 char num16k; //number of 16k rom banks
 char num8k; //number of 8k rom banks

 char mirroring:1;//type of mirroring (1-vertical
 char battery:1;  //battery backed rom?
 char trainer:1;  //trainer?
 char reservedbit:1;          //reserved
 char banktype:4; //type of bank selector

 char reserved[9];
 void printinfo();
 int validate();
};

//memory mapper types
#define MMC_NONE   0
#define MMC_SEQ    1
#define MMC_KONAMI 2
#define MMC_VROMSWITCH 3
#define MMC_5202   4

typedef byte ROMPAGE[0x4000];  //one page of ROM
typedef byte VROMPAGE[0x2000]; //one page of VROM


extern NESROMHEADER romhdr;
extern char *trainer;
extern int numrom,numvrom; //number of rom/vrom pages
extern ROMPAGE  *ROM;  //array of 16k rom pages
extern VROMPAGE *VROM; //array of 8k vrom pages

extern char *ram; //CPU address space
extern struct NES_ppumemory *ppu; //PPU address space
extern struct NES_sprite *spritemem; //Sprite memory

extern int NESinitialized;
extern int romloaded;
void initNEShardware();           //create nes hardware
void terminateNEShardware();      //destroy
int resetNEShardware();           //reset and setup roms and mappers

int getscanline(); //get virtual scanline
extern byte CPUpaused,CPURunning;

void loadrom(char *romfile);
void freerom();
void m_execute();
void m_stop();

#endif
