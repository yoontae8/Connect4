#ifndef _LOGIC_
#define _LOGIC_

#include <stdio.h>

#define RED 1
#define YELLOW 2 

int insert(int (*table)[7], int position, int player);
int check(int (*table)[7], int xpos, int ypos, int player);
int checkVertical(int (*table)[7], int xpos, int ypos, int player);
int checkHorizontal(int (*table)[7], int xpos, int ypos, int player);
int checkDiagonal1(int (*table)[7], int xpos, int ypos, int player);
int checkDiagonal2(int (*table)[7], int xpos, int ypos, int player);
int getWeight(int (*table)[7], int xpos, int ypos, int player);
int wCheckVertical(int (*table)[7], int xpos, int ypos, int player);
int wCheckHorizontal(int (*table)[7], int xpos, int ypos, int player);
int wCheckDiagonal1(int (*table)[7], int xpos, int ypos, int player);
int wCheckDiagonal2(int (*table)[7], int xpos, int ypos, int player);
   
#endif   

