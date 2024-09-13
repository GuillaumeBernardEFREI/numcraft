#include "eadkpp.h"

signed char _round(double x){
  return (signed char)x -(x<0);
}

double abs(double x){
    return (x<0)? -x :x;
}
double sin(double angle);

unsigned int randint(unsigned int a,unsigned int b){
  unsigned int rand;
  unsigned int c= b-a;
  do{
  unsigned int c_ =c;
  unsigned char n=0;
  while (c_!=0)
  {
    c_=c_>>1;
    n++;
  }
   rand= EADK::random()>>(32-n);
  }while (rand>c);
  return rand+a;
  
  
  
}

double cos(double angle){
    return sin(angle +1.57079632679);
}
double sin(double angle){
    bool invertSign = false;
    
    if(abs(angle)<0.3){
      return angle;
    }
    if (angle>1.57079632679){
        return -sin(angle - 3.14159265359);
    }
    if (angle<-1.57079632679){
        return -sin(angle + 3.14159265359);
    }
    /*
    //change to while if sin stop working.
    if (angle>1.57079632679){
        invertSign =!invertSign;
         angle -= 3.14159265359;
    }
    //same here
    if(angle<-1.57079632679){
        invertSign =!invertSign;
        angle+= 3.14159265359;
    }
    */



    //create a power func?
    /*
    less accurate but faster
    double s= angle-angle*angle*angle*0.166666666667;
    */


    double x2=angle*angle;
    double x3=x2*angle;
    double x5=x2*x3;


    double s= angle-x3*0.166666666667+x5*0.00833333333333;

    if (abs(s-1)<0.0045){
        return (s<0)? -1:1;
    }
    
    return s;
}
double q_rsqrt(double number)
{ 

  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = (float)number * 0.5F;
  y  = (float)number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return (double)y;
}
double max(double a, double b)
{
  return (a > b) ? a : b;
}
double min(double a, double b)
{
  return (a < b) ? a : b;
}