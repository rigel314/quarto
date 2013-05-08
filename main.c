/*
 * main.c
 *
 *  Created on: Nov 18, 2012
 *      Author: cody
 *
 *
 *  TODO: Win detection.
 *  TODO: Player indication.
 */

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

#define boardWidth		10
#define boardHeight		5
#define boardStartRow	5
#define boardStartCol	0
#define HORIZ			0
#define VERT			1
#define centerFactH		(boardWidth-3)/2+1
#define centerFactV		(boardHeight-2)/2+1
#define pieceMsg		"Select a piece."
#define placeMsg		"Place piece %c. "
#define player1Msg		"Player 1:"
#define player2Msg		"Player 2:"
#define winsMsg			"Wins           "
#define gameOverMsg		"Game Over !"

typedef union
{
	struct
	{
		unsigned char height:1;
		unsigned char shape:1;
		unsigned char divet:1;
		unsigned char color:1;
	};
	int attrs;
} piece;

void printBoard(piece board[4][4]);
void printBoardPiece(int row, int col, piece p);
void printLine(int row, int col, int direction, int len);
void printPiece(int row, int col, piece p);
char toHex(int x);
int fromHex(char x);
int fullRows(piece board[4][4]);
int orPiece(piece p);
int pathWinP(piece* p[4]);
piece** makePath(piece board[4][4], int pathNum);
int isWin(piece board[4][4]);
void safeExit(int ret);

piece board[4][4];
piece blank;

int main()
{
	piece pieces[16];
	char used[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int i,c,last;
	char select = 1;
	bool player = true;
	bool gameOver = false;
	piece* newPiece;

	blank.attrs = -1;
	for(i=0;i<16;i++)
	{
		pieces[i].attrs=i;
		board[i/4][i%4].attrs=-1;
	}

	// ncurses stuff
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr,TRUE);

	// Pieces in the pool.
	for(i=0;i<16;i++)
	{
		mvprintw(0,5*i,"%x",i);
		printPiece(1,5*i,pieces[i]);
	}

	mvprintw(boardStartRow+boardHeight*4+1,0,player1Msg);
	mvprintw(boardStartRow+boardHeight*4+1,sizeof(player1Msg),pieceMsg);
	printBoard(board);
	refresh();

	while((c = getch()))
	{
		if(((c>='0' && c<='9') || (c>='a' && c<='f')) && !gameOver)
		{
			i = fromHex(c);
			if(select && !used[i])
			{
				select = !select;
				player = !player;
				used[i] = 1;
				newPiece = &pieces[i];

				attron(A_BOLD);
				printPiece(1,5*i,pieces[i]);
				attroff(A_BOLD);

				last = i;
			}
			else if(!select && board[i/4][i%4].attrs == -1)
			{
				select = !select;
				board[i/4][i%4].attrs = newPiece->attrs;

				mvaddch(0,5*last,' ');
				printPiece(1,5*last,blank);
			}
		}
		if(c==KEY_EOL || c==KEY_EXIT || c==10 || c==13)
		{
			endwin();
			return 0;
		}
		mvprintw(boardStartRow+boardHeight*4+1,0,((player) ? player1Msg : player2Msg));
		mvprintw(boardStartRow+boardHeight*4+1,((player) ? sizeof(player1Msg) : sizeof(player2Msg)),((select) ? pieceMsg : placeMsg),toHex(i));
		printBoard(board);
		// DEBUG SHIT HERE. mvprintw(0,0,"%d",fullRows(board));
		if(isWin(board))
		{
			gameOver = true;
			mvprintw(boardStartRow+boardHeight*4+1,((player) ? sizeof(player1Msg) : sizeof(player2Msg)),winsMsg);
			mvprintw(boardStartRow+boardHeight*4+1+1,0,gameOverMsg);
		}
		refresh();
	}

	endwin();
	return 0;
}

void printBoard(piece board[4][4])
{
	int i,j;

	//******************************** Top Row ********************************************************************
	mvvline(boardStartRow,boardStartCol,ACS_ULCORNER,1);														//*
	printLine(boardStartRow,boardStartCol+1,HORIZ,boardWidth-1);												//*
	for(i=1;i<4;i++)																							//*
	{																											//*
		mvvline(boardStartRow,boardStartCol+boardWidth*i,ACS_TTEE,1);											//*
		printLine(boardStartRow,boardStartCol+boardWidth*i+1,HORIZ,boardWidth-1);								//*
	}																											//*
	mvvline(boardStartRow,boardStartCol+boardWidth*4,ACS_URCORNER,1);											//*
	//*************************************************************************************************************

	//******************************** Middle Rows ****************************************************************
	for(i=0;i<4;i++)																							//*
	{																											//*
		for(j=0;j<5;j++)																						//*
		{																										//*
			printLine(boardHeight*i+boardStartRow+1,boardStartCol+boardWidth*j,VERT,boardHeight-1);				//*
		}																										//*
		mvvline(boardHeight*i+boardStartRow+boardHeight,boardStartCol,ACS_LTEE,1);								//*
		printLine(boardHeight*i+boardStartRow+boardHeight,boardStartCol+1,HORIZ,boardWidth-1);					//*
		for(j=1;j<4;j++)																						//*
		{																			 							//*
			mvvline(boardHeight*i+boardStartRow+boardHeight,boardStartCol+boardWidth*j,ACS_PLUS,1);				//*
			printLine(boardHeight*i+boardStartRow+boardHeight,boardStartCol+boardWidth*j+1,HORIZ,boardWidth-1);	//*
		}																										//*
		mvvline(boardHeight*i+boardStartRow+boardHeight,boardStartCol+boardWidth*4,ACS_RTEE,1);					//*
	}																											//*
	//*************************************************************************************************************

	//******************************** Bottom Row *****************************************************************
	mvvline(boardStartRow+boardHeight*4,boardStartCol,ACS_LLCORNER,1);											//*
	printLine(boardStartRow+boardHeight*4,boardStartCol+1,HORIZ,boardWidth-1);									//*
	for(i=1;i<4;i++)																							//*
	{																											//*
		mvvline(boardStartRow+boardHeight*4,boardStartCol+boardWidth*i,ACS_BTEE,1);								//*
		printLine(boardStartRow+boardHeight*4,boardStartCol+boardWidth*i+1,HORIZ,boardWidth-1);					//*
	}																											//*
	mvvline(boardStartRow+boardHeight*4,boardStartCol+boardWidth*4,ACS_LRCORNER,1);								//*
	//*************************************************************************************************************

	//******************************** Labels *********************************************************************
	for(i=0;i<4;i++)																							//*
	{																											//*
		for(j=0;j<4;j++)																						//*
		{																										//*
			if(board[i][j].attrs == -1)																			//*
				mvaddch(boardStartRow+boardHeight*i+1,boardStartCol+boardWidth*j+1,toHex(4*i+j));				//*
			else																								//*
				mvaddch(boardStartRow+boardHeight*i+1,boardStartCol+boardWidth*j+1,' ');						//*
			printBoardPiece(i,j,board[i][j]);																	//*
		}																										//*
	}																											//*
	//*************************************************************************************************************
}

void printLine(int row, int col, int direction, int len)
{
	int i;
	switch (direction) {
		case HORIZ:
			for(i=0;i<len;i++)
				mvhline(row,col+i,ACS_HLINE,1);
			break;
		case VERT:
			for(i=0;i<len;i++)
				mvvline(row+i,col,ACS_VLINE,1);
			break;
		default:
			break;
	}
}

void printBoardPiece(int row, int col, piece p)
{
	printPiece(boardStartRow+row*boardHeight+centerFactV,boardStartCol+col*boardWidth+centerFactH,p);
}

void printPiece(int row, int col, piece p)
{
	if(p.attrs != -1)
	{
		mvprintw(row,col,"%d",p.color);
		mvprintw(row,col+2,"%d",p.divet);
		mvprintw(row+1,col,"%d",p.shape);
		mvprintw(row+1,col+2,"%d",p.height);
	}
	else
	{
		mvprintw(row,col," ");
		mvprintw(row,col+2," ");
		mvprintw(row+1,col," ");
		mvprintw(row+1,col+2," ");
	}
}

char toHex(int x)
{
	if(x>=0 && x<=9)
		return '0'+x;
	else
		return 'a'+x-10;
}

int fromHex(char x)
{
	if(x>='0' && x<='9')
		return x-'0';
	else
		return x-'a'+10;
}

int fullRows(piece board[4][4])
{ // Output 10bit number with each bit showing whether or not a path is complete(should be checked for end of game).
	int i,j;
	int out = 1023;

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{ // Yay! Mixing logical and bitwise operations.
			// So, if there is no piece, shift the 1 up by i and complement it.
			// Then AND with out.  This only changes bit i.

			out = ~((board[i][j].attrs == -1) << i) & out; // Horizontals

			out = ~((board[i][j].attrs == -1) << (4+j)) & out; // Verticals -- easier than I was expecting.

			if(i==j) // For our diagonals too.
				out = ~((board[i][j].attrs == -1) << 8) & out;
			if(i==3-j)
				out = ~((board[i][j].attrs == -1) << 9) & out;
		}
	}
	return out;
}
int orPiece(piece p)
{
	return p.color<<3 | p.divet<<2 | p.shape<<1 | p.height;
}
int pathWinP(piece* p[4])
{
	if(!p)
		return 0;
	return ~((p[0]->attrs & p[1]->attrs & p[2]->attrs & p[3]->attrs) ^ (p[0]->attrs | p[1]->attrs | p[2]->attrs | p[3]->attrs)) & 15;
}
piece** makePath(piece board[4][4], int pathNum)
{
	int i;
	piece** path;

	path = calloc(4,sizeof(piece*));

	if(!path)
		return NULL;

	for(i=0; i<4; i++)
	{
		int X;
		int Y;

		if(pathNum<4){
			X=pathNum;
			Y=i;}
		else if (pathNum<8){
			X=i;
			Y=pathNum%4;}
		else if (pathNum==8){
			X=i;
			Y=i;}
		else if (pathNum==9){
			X=i;
			Y=3-i;
		}

		path[i] = &board[X][Y];
	}

	return path;
}
int isWin(piece board[4][4])
{
	int i,j;
	int wins;
	piece** p;

	for(i=fullRows(board), j=0; i!=0; i >>= 1, j++)
	{
		if(!(i & 1)) // If the path isn't full.
			continue;

		p = makePath(board,j);

		wins = pathWinP(p);
		free(p);

		if(wins)
		{
			//mvprintw(j,0,"%d:%d",j,wins);
			return 1;
		}
	}

	return 0;
}

void safeExit(int ret)
{
	endwin();
	exit(ret);
}
