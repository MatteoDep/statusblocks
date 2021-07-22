typedef struct {
	char* icon;
	char* command;
	unsigned int interval;
	unsigned int signal;
} Block;
//Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
	/*Icon*/	/*Command*/		/*Update Interval*/	/*Update Signal*/
	{"%{l}",  "sb-bspwm",     0,  1},
	{"",      "sb-wtitle",    0,  1},
	{"%{c}",  "sb-clock",     0,  2},
	{"%{r}",  "sb-news",      0,  6},
	{"",      "sb-crypto",    0,  13},
	{"",      "sb-volume",    0,  10},
	{"",      "sb-battery",   5,  3},
	{"",      "sb-internet",  5,  4},
};

//sets delimeter between status commands. NULL character ('\0') means no delimeter.
static char delim[] = " ";
