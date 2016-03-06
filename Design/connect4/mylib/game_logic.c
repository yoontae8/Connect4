#include "logic_header.h"

//find the empty(0) position in the column and insert(change to) player
//return row number
int insert(int (*table)[7], int position, int player) {
    int i = 5;
    while(table[i][position] != 0) {
	i--;
	if(i < 0)
	    return -1;
    }
    table[i][position] = player;

    return i;
}

//check if player is win
int check(int (*table)[7], int xpos, int ypos, int player) {
    int count, i;
    // there is line with four markers connected
    if(checkVertical(table, xpos, ypos, player) == 0 ||
	checkHorizontal(table, xpos, ypos, player) == 0 ||
	checkDiagonal1(table, xpos, ypos, player) == 0 ||
	checkDiagonal2(table, xpos, ypos, player) == 0)

	return 0;

    //board is full
    for(count = 0, i = 0; i < 7; i++)
	if(table[0][i] != 0)
	    count++;
    if(count == 7)
	return 1;

    //nothing 
    else
	return -1;

}
//check vertically
int checkVertical(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0; //connected number
    int i, begin, end;
    
    //count the marker in the left
    i = xpos;
    while(table[ypos][i--] == player) {
	count++;
	if(i < 0)
	    break;
    }
    begin = i+1;

    //count the marker in the right 
    i = xpos;
    while(table[ypos][i++] == player) {
	count++;
	if(i > 6)
	    break;
    }
    end = i-1;

    //if winning condition meets
    if(count > 4) {
	count = 0;
	for(i = begin; i <= end; i++) {
	    //set the factor of win to '3'
	    if(table[ypos][i] == player)
	    {
		count++;
		table[ypos][i] = 3;
	    }
	    if( count == 4)
		break;
	}
	return 0; 
    }

    return -1;
}

//check horizontally
int checkHorizontal(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i, begin, end;
    
    //count the marker in the upside
    i = ypos;
    while(table[i--][xpos] == player) {
	count++;
	if(i < 0)
	    break;
    }
    begin = i;

    //count the marker in the downside 
    i = ypos;
    while(table[i++][xpos] == player) {
	count++;
	if(i > 5)
	    break;
    }
    end = i;
    if(count > 4) {
	count = 0;
	for(i = begin; i <= end; i++) {
	    if(table[i][xpos] == player)
	    {
		count++;
		table[i][xpos] = 3;
	    }
	    if( count == 4)
		break;
	}
    
	return 0;
    }

    return -1;
}

//check diagonally(\)
int checkDiagonal1(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i, j, begi, begj, endi, endj;

    //count marker in the left-upper side
    i = xpos, j = ypos;
    while(table[j--][i--] == player) {
	count++;
	if(i < 0 ||j < 0)
	    break;
    }
    begi = i + 1;
    begj = j + 1;

    //count marker in the left-lower side
    i = xpos, j = ypos;
    while(table[j++][i++] == player) {
	count++;
	if(i > 6 || j > 5)
	    break;
    }
    endi = i - 1;
    endj = j - 1;
    
    if(count > 4) {
	count = 0;
	for(i = begi, j = begj; i <= endi, j <= endj; i++, j++) {
	    if(table[j][i] == player)
	    {
		count++;
		table[j][i] = 3;
	    }
	    if( count == 4)
		break;
	}
	return 0;
    }

    return -1;
}

//check diagonally(/)
int checkDiagonal2(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i, j, begi, begj, endi, endj;

    //count marker in the left-lower side
    i = xpos, j = ypos;
    while(table[j++][i--] == player) {
	count++;
	if(i < 0 ||j > 5)
	    break;
    }
    begi = i;
    begj = j; 

    //count marker in the right-upper side
    i = xpos, j = ypos;
    while(table[j--][i++] == player) {
	count++;
	if(i > 6 || j < 0)
	    break;
    }
    endi = i;
    endj = j;

    if(count > 4) {
	count = 0;
	for(i = begi, j = begj; i <= endi, j >= endj; i++, j--) {
	    if(table[j][i] == player)
	    {
		count++;
		table[j][i] = 3;
	    }
	    if( count == 4)
		break;
	}
	return 0;
    }

    return -1;

	    
}

//get weight of the given position
int getWeight(int (*table)[7], int xpos, int ypos, int player) {
    int val1, val2, val3, val4, winner1, winner2;
    //get each weight of the lines for player
    val1 = wCheckVertical(table, xpos, ypos, player);
    val2 = wCheckHorizontal(table, xpos, ypos, player);
    val3 = wCheckDiagonal1(table, xpos, ypos, player);
    val4 = wCheckDiagonal2(table, xpos, ypos, player);

    //get biggest weight among lines for player
    winner1 = val1;
    winner1 = winner1 >= val2 ? winner1 : val2;
    winner1 = winner1 >= val3 ? winner1 : val3;
    winner1 = winner1 >= val4 ? winner1 : val4;

    //if it is possible to win
    if(winner1 >= 4)
	return 9;

    //get each weight of the lines for opponent 
    val1 = wCheckVertical(table, xpos, ypos, player%2+1);
    val2 = wCheckHorizontal(table, xpos, ypos, player%2+1);
    val3 = wCheckDiagonal1(table, xpos, ypos, player%2+1);
    val4 = wCheckDiagonal2(table, xpos, ypos, player%2+1);

    //get biggest weight among lines for opponent
    winner2 = val1;
    winner2 = winner2 >= val2 ? winner2 : val2;
    winner2 = winner2 >= val3 ? winner2: val3;
    winner2 = winner2 >= val4 ? winner2: val4;

    //if it is possible to lose
    if(winner2 >= 4)
	return 8;

    return winner1 + winner2;

}
//return the number of markers beside the position vertically
int wCheckVertical(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i;
    
    i = xpos-1;
    while(table[ypos][i--] == player) {
	count++;
	if(i < 0)
	    break;
    }

    i = xpos+1;
    while(table[ypos][i++] == player) {
	count++;
	if(i > 6)
	    break;
    }
    return count + 1;
}

//return the number of markers beside the position horizontally 
int wCheckHorizontal(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i;
    
    i = ypos-1;
    while(table[i--][xpos] == player) {
	count++;
	if(i < 0)
	    break;
    }

    i = ypos+1;
    while(table[i++][xpos] == player) {
	count++;
	if(i > 5)
	    break;
    }
    return count + 1;
}

//return the number of markers beside the position diagonally(\)
int wCheckDiagonal1(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i, j;

    i = xpos-1, j = ypos-1;
    while(table[j--][i--] == player) {
	count++;
	if(i < 0 ||j < 0)
	    break;
    }

    i = xpos+1, j = ypos+1;
    while(table[j++][i++] == player) {
	count++;
	if(i > 6 || j > 5)
	    break;
    }
    
    return count+1 ;
}

//return the number of markers beside the position diagonally(/)
int wCheckDiagonal2(int (*table)[7], int xpos, int ypos, int player) {
    int count = 0;
    int i, j;

    i = xpos+1, j = ypos-1;
    while(table[j++][i--] == player) {
	count++;
	if(i < 0 ||j > 5)
	    break;
    }

    i = xpos-1, j = ypos+1;
    while(table[j--][i++] == player) {
	count++;
	if(i > 6 || j < 0)
	    break;
    }

    return count+1 ;

	    
}
