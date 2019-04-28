#ifndef _TYPES_
#define _TYPES_

typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned short word;


//short point
struct point { short int x,y;};

//long point
struct lpoint  {int x,y;};

//short 3d point
struct zdpoint
{
 short int x;
   signed short z:15;
 unsigned short d:1;
 short int y;
 void clear() {x=0; z=0; y=0; d=0;}
} ;

struct zpoint
{
 short int x,z,y;
 void clear() {x=0; z=0; y=0;}
};

//long point
struct lzpoint  {int x,y,z;};

//rectangle (short)
struct rect
{
signed short int x1,y1,x2,y2;
};

//3d rectangle (short)
struct zrect
{
 zpoint p1,p2;
 void clear() {p1.clear(); p2.clear();}
 void reorient()
  {
   if (p1.x>p2.x) {int temp=p2.x; p2.x=p1.x; p1.x=temp;}
   if (p1.y>p2.y) {int temp=p2.y; p2.y=p1.y; p1.y=temp;}
   if (p1.z>p2.z) {int temp=p2.z; p2.z=p1.z; p1.z=temp;}
  }
};


//rectangle (long)
struct lrect
{
 int x1,y1,x2,y2;
 int width() {return x2-x1;}
 int height() {return y2-y1;}

};

#endif
