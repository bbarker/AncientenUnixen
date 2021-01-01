/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#define INCH 240
/*
DASI450
nroff driving tables
width and code tables
*/

struct {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	int zzz;
	} t = {
/*bset*/	0,
/*breset*/	0177420,
/*Hor*/		INCH/60,
/*Vert*/	INCH/48,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033R",
/*twrest*/	"\033R",
/*twnl*/	"\015\n",
/*hlr*/		"\033S",
/*hlf*/		"\033B",
/*flr*/		"\033\n",
/*bdon*/	"",
/*bdoff*/	"",
/*ploton*/	"\0334",
/*plotoff*/	"\033R",
/*up*/		"\005",
/*down*/	"\n",
/*right*/	" ",
/*left*/	"\b",
/*codetab*/
#include "code.300"
