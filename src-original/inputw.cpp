//allows for changing of input devices
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "input.h"
#include "dd.h"
#include "r2img.h"
#include "font.h"

//#include "object.h"
//#include "objdef.h"

#include "keyb.h"

#include "config.h"

//input device settings.
extern inputdevicesettings *ids;

#define NUMINPUTTYPES 6
char *inputnames[]=
 {
  "None",
  "Gravis Gamepad",
  "Keyboard 1",
  "Keyboard 2",
  "Joystick 1",
  "Joystick 2",
  "GrIP Slot 1",
  "GrIP Slot 2",
 };




class keyedit:public GUIedit
{
 char *keyname;
 char &keyscan; //scancode to be modified
 public:
 keyedit(GUIrect *p,char *pmpt,int x2,int y,char &scan)
   :GUIedit(p,pmpt,x2,y,70),keyscan(scan)
   {
    moverel(-width(),0);
    keyname=getkeyname(keyscan);
   };

 virtual GUIrect *click(mouse &m)
  {
   if (modal) setmodal(0);  else setmodal(this);
   setfocus(this);
   return 0;
  };

 virtual int release(mouse &m) {return 1;}

 virtual int keyhit(char kbscan,char key)
  {
   if (modal!=this)
    {
     if (kbscan==KB_ENTER) {setmodal(this); return 1;}
     setmodal(0);
     return 0;
    }
   if (kbscan!=KB_ESC)
   {
    keyscan=kbscan;
    keyname=getkeyname(keyscan);
    parent->cyclefocus(0);
   }
   setmodal(0); //parent->parent);
   return 1;
  }

 virtual void drawdata(char *dest,int x,int y,int xw)
   {
    if (modal==this && (uu&16)) drawrect(dest,2,x,y,xw,10);
    font[1]->draw(keyname,dest,x,y);
   }

 virtual char *getname() {return "keyedit";}
};

char *mapnames[]={
     "Up-left", //0
     "Up",
     "Up-right",
     "Left",
     "Right",
     "Down-left", //5
     "Down",
     "Down-Right",
     "Button 1", //8
     "Button 2",
     "Button 3",
     "Button 4", //11
};


class redefinekeys:public GUIcontents
{
keymap &km;

public:
redefinekeys(keymap &tkm):GUIcontents(150,150),km(tkm)
 {
  for (int i=0; i<=11; i++)
  {
   char s[30];
   strcpy(s,mapnames[i]);
   strcat(s,": ");
   new keyedit(this,s,width()-5,i*12+3,((char *)&km)[i]);
  }
 };

 virtual ~redefinekeys()
  {
   pos.close((GUIbox *)parent);
   losefocus();
  }

 virtual int acceptfocus() {return 1;}

 virtual void draw(char *dest)
 {
  fill(1);
  GUIrect::draw(dest);
 }

 virtual char *getname() {return "redefinekeys";}

 //open functions
 static DLGPOS pos; //saved last position of dialog
 static void open(keymap *km)
  {
   pos.open(new GUIbox(guiroot,"Redefine keys",new redefinekeys(*km),0,0));
//   setmodal(pos.opened);
  };
};
DLGPOS redefinekeys::pos;





typedef input *INPUTPTR;
class inputtestbutton:public GUIrect
{
 input *in;
 int button; //button number
 public:

 void setinput(input *tin) {in=tin;}
 void setbutton(int tbut) {button=1<<(tbut+4);}

 inputtestbutton(GUIrect *p,int x1,int y1,int xw,int yw,input *tin,int tbut)
  :GUIrect(p,x1,y1,x1+xw,y1+yw)
   {
    setinput(tin); setbutton(tbut);
   }
 virtual void draw(char *dest)
  {
   if (in && in->but&button) fill(4);
                  else fill(9*16+14);
   outline(4);
  }
 virtual char *getname() {return "inputtestbutton";}
};

class inputtestarea:public GUIrect
{
 input *in; //input to test
 public:
 void setinput(input *tin) {in=tin;}
 inputtestarea(GUIrect *p,int x1,int y1,int width,input *tin):
   GUIrect(p,x1,y1,x1+width,y1+width)
   {
    setinput(tin);
   }

 virtual void draw(char *dest)
  {
   //drawbox(dest,4,x1-1,y1-1,x2+1,y2+1);
   fill(9*16+14);
   outline(4);
   if (!in) return;

   int x,y;
   if (in->stat&ID_RIGHT) x=width()-10; else
   if (in->stat&ID_LEFT) x=0; else  x=width()/2-5;
   if (in->stat&ID_DOWN) y=height()-10; else
   if (in->stat&ID_UP) y=0; else  y=height()/2-5;

   drawrect(dest,2,x1+x,y1+y,10,10);
  }
 virtual char *getname() {return "inputtestarea";}
};

#include "message.h"

class changeinputdevice:public GUIcontents
{
 protected:
 int lastintype;
 int inputnum;
 INPUTPTR &in; //pointer to input device being changed
 GUIstringlistbox *list;
 inputtestarea *test;
 inputtestbutton *b[4];

 GUItextbutton *edit,*save;

 object *lasto; //object that was bound to input


 public:
 changeinputdevice(int num,INPUTPTR &tin)
 :GUIcontents(220,135),in(tin),inputnum(num)
 {
  new GUIstatictext(this,1,"Input type:",5,3);
  list=new GUIstringlistbox(this,5,15,110,NUMINPUTTYPES,10);
  edit=save=0;
   //fill list box
  ITEMPTR *a=list->resizeitems(NUMINPUTTYPES);
  for (int i=0; i<NUMINPUTTYPES; i++) a[i]=(ITEMPTR)inputnames[i];

  new GUIstatictext(this,1,"Test area:",125,3);
  test=new inputtestarea(this,125,15,91,in);
  for (i=0; i<4; i++) b[i]=new inputtestbutton(this,127+i*23,110,18,10,in,i);
  lasto=0;
//  msg.printf(2,"in=%p in->o=%p",in,in ? in->o : 0);
  if (in)
   {
    lasto=in->o;
    list->setsel(in->type);
    lastintype=in->type;
//    in->bind(0);
   }
//  msg.printf(2,"in=%p in->o=%p lasto=%p",in,in ? in->o : 0,lasto);

 }

 void changedevice(int type)
 {
  disable();
  if (in) delete in;
  in=newinputdevice(type);
  in->bind(lasto);
  enable();
 }

 virtual ~changeinputdevice()
  {
   pos.close((GUIbox *)parent);
   if (in)
    {
     changedevice(lastintype); //reset input device from last settings
//     in->bind(lasto);
    }
  }

 virtual void losefocus()
 {
  GUIrect::losefocus();
 }

 virtual void receivefocus()
 {
  if (in) in->testing=1;
  GUIrect::receivefocus();
 }

 virtual int sendmessage(GUIrect *c,int guimsg)
  {
   if (c==edit)
    if (guimsg==GUIMSG_PUSHED)
    if (in)
    {
     if (in->getkeymap())
      {
        redefinekeys::open(in->getkeymap());
      }
     //else  if (in->getjoythreshold()) calibratejoy::open(in->getjoythreshold());
     in->testing=0;
    }
   if (c==save)
    if (guimsg==GUIMSG_PUSHED)
    {
     if (in)
      {
       in->savesettings(); //save settings to ids
       lastintype=in->type;
       cfg->pinput[inputnum]=in->type;
       cfg->save(configfile);
      }
     delete parent;
     return 1;
    }

   if (guimsg==GUIMSG_LISTBOXSELCHANGED) //they changed input devices
    {
     int x=list->getselnum();
     if (x<0)  return 1;
     changedevice(x);
     if (in) in->testing=1;

     if (test)
     {
      test->setinput(in);
      for (int i=0; i<4; i++) b[i]->setinput(in);
     }

     if (edit) {delete edit; edit=0;}
     if (save) {delete save; save=0;}
     if (in)
      {
       if (in->getjoythreshold()) edit=new GUItextbutton(this,"Calibrate...",60,90); else
       if  (in->getkeymap())   edit=new GUItextbutton(this,"Redefine keys...",60,90);
      }
     //if (edit)
      save=new GUItextbutton(this,"Save settings",60,105);
    }
   return 1;
  };

 virtual int acceptfocus() {return 1;}
 virtual char *getname() {return "changeinputdevice";}
 virtual void draw(char *dest)
  {
   fill(1);
/*   if (lasto) //is bound?
   {
    char s[80];
    sprintf(s,"Bound to: %s",lasto->od->name);
    font[3]->drawcentered(s,dest,x1+width()/2,y1+125);
   }*/

   GUIrect::draw(dest);
  };

 //open functions
 static DLGPOS pos; //saved last position of dialog
 static void open(int inputdevicenum)
  {
   if (pos.opened) return;
    char s[80];
    sprintf(s,"Input device %d",inputdevicenum+1);
    pos.open(new GUIbox(guiroot,s,new changeinputdevice(inputdevicenum,inputdevice[inputdevicenum]),0,0));
  };
};
DLGPOS changeinputdevice::pos;

void editinputdevice0() {changeinputdevice::open(0);}
void editinputdevice1() {changeinputdevice::open(1);}




