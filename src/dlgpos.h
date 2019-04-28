
extern "C" int SCREENX,SCREENY; 


//class for positioning new dialog boxes and saving their coordinates
//for later instances
class DLGPOS
{
 int x,y;       //position of dialog box
 int findnewpos; //should we allocate a new pos or use the saved position?

 public:
 class GUIbox *opened; //pointer to last opened dialog box (0 for none)
 DLGPOS() {findnewpos=1; opened=0;}

 void getnewpos(GUIbox *t)
 {
  if (findnewpos)  
  {
  //get coords of where dialog SHOULD go
  x=GUIdefx; y=GUIdefy;
  nextGUIdef();
  }

  //if dlg is going off screen...
  if (x+t->width()>SCREENX-20 || y+t->height()>SCREENY-20)
   {
    resetGUIdef();
    x=GUIdefx; y=GUIdefy;
     //if STILL going off screeen....
    if (x+t->width()>SCREENX-20 || y+t->height()>SCREENY-20)
     {
      x=(SCREENX-t->width())/2; if (x<0) x=0;
      y=(SCREENY-t->height())/2; if (y<0) y=0;
     }
   }
 }
  //opens a new dialog box, repositions
 void open(GUIbox *t)
  {
   //already opened
   if (opened) {delete t; GUIrect::setfocus(opened); return;} //set focus to old one
   getnewpos(t);
   t->moverel(x,y); //position it on screen
   opened=t; //set it as opened
  }
 void close(GUIbox *t)
 {
  if (t!=opened) return;
  x=t->x1; y=t->y1; //save dialog position for later
  findnewpos=0;
  opened=0;
 }
 void setxy(int nx,int ny)
  {
   x=nx; y=ny; findnewpos=0;
  }
};

