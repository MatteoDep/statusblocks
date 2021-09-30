#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __OpenBSD__
#define SIGPLUS SIGUSR1 + 1
#define SIGMINUS SIGUSR1 - 1
#else
#define SIGPLUS SIGRTMIN
#define SIGMINUS SIGRTMIN
#endif
#define MAXCMDLENGTH 1000
#define MAXLINELENGTH 500

#ifndef __OpenBSD__
void dummysighandler(int num);
#endif
void sighandler(int num);
void getcmds(int time);
void getsigcmds(unsigned int signal);
void setupsignals();
void sighandler(int signum);
int getstatus(char *str, char *last);
void statusloop();
void termhandler();
void writestatus();

typedef struct {
  unsigned int interval;
  unsigned int signal;
  char command[MAXLINELENGTH];
} Block;

int block_num = 0;
Block *blocks;
char **delims;
static char **statusbar;
static char *statusstrold;
static char *statusstrnew;
static int statusContinue = 1;
static int returnStatus = 0;

void trim(char *line) {
  // strips away leading and trailing spaces
  int start = 0;
  int len = strlen(line);
  int end = len;
  unsigned int startdone = 0;
  unsigned int enddone = 0;

  while (((!startdone) || (!enddone))) {
    if ((line[start] != ' ') && (line[start] != '\t'))
      startdone = 1;
    else if (!startdone)
      start++;
    if ((line[end-1] != ' ') &&
        (line[end-1] != '\t') &&
        (line[end-1] != '\n') &&
        (line[end-1] != '\0'))
      enddone = 1;
    else if (!enddone)
      end--;
    if (start > end)
      start = end;
  }
  strncpy(line, line+start, end-start);
  line[end-start] = '\0';
}

void cfgline2block(Block *block, char *line) {
  char interval[5];
  char signal[5];
  unsigned short int isprevspace = 0;
  unsigned short int count = 0;
  unsigned short int start = 0;
  for (int i = 0; i < strlen(line); i++) {
    if ((line[i] == ' ') || (line[i] == '\t')) {
      if (!isprevspace) {
        if (count == 0)
          strncpy(interval, line + start, i - start);
        else if (count == 1)
          strncpy(signal, line + start, i - start);
      }
      isprevspace = 1;
    } else {
      if (isprevspace) {
        count++;
        start = i;
        if (count == 2)
          strcpy(block->command, line + start);
        isprevspace = 0;
      }
    }
  }
  block->interval = atoi(interval);
  block->signal = atoi(signal);
}

void allocate(int block_num) {
  // TODO realloc strings and check alloc successfull
  // TODO refactor alloc in functions
  if (block_num==0) {
    blocks = (Block*) malloc(sizeof(Block));
    delims = (char**) malloc(sizeof(char*));
    delims[block_num] = (char*) malloc(MAXLINELENGTH*sizeof(char));
    statusbar = (char**) malloc(sizeof(char*));
    delims[block_num][0] = '\0';
    statusstrold = (char*) malloc((block_num+1)*MAXLINELENGTH);
    statusstrnew = (char*) malloc((block_num+1)*MAXLINELENGTH);
  } else {
    if (block_num>1)
      blocks = realloc(blocks, block_num*sizeof(Block));
      statusbar = realloc(statusbar, block_num*sizeof(char*));
    delims = realloc(delims, (block_num+1)*sizeof(char*));
    delims[block_num] = (char*) malloc(MAXLINELENGTH*sizeof(char));
    delims[block_num][0] = '\0';
    statusbar[block_num-1] = (char*) malloc(MAXCMDLENGTH*sizeof(char));
    statusbar[block_num-1][0] = '\0';
    statusstrold = realloc(statusstrold, (block_num+1)*MAXLINELENGTH + block_num*MAXCMDLENGTH);
    statusstrnew = realloc(statusstrnew, (block_num+1)*MAXLINELENGTH + block_num*MAXCMDLENGTH);
  }
}

void free_memory() {
  free(blocks);
  free(statusstrold);
  free(statusstrnew);
  for (int i=0; i<block_num; i++) {
    free(delims[i]);
    free(statusbar[i]);
  }
  free(delims);
  free(statusbar);
}

void parseconfig(char *configfile) {
  FILE *config = fopen(configfile, "r");
  if (!config) {
    printf("config file %s not found.\n", configfile);
    exit(EXIT_FAILURE);
  }

  allocate(block_num);
  char buf[MAXLINELENGTH];
  while (fgets(buf, MAXLINELENGTH, config) != NULL) {
    trim(buf);
    switch (buf[0]) {
      case '\0':
        break;
      case '#':
        break;
      case '\"':
        strncat(delims[block_num], buf + 1, strlen(buf) - 2);
        break;
      default:
        block_num++;
        allocate(block_num);
        cfgline2block(blocks+(block_num-1), buf);
    }
  }
  fclose(config);
}

// opens process *cmd and stores output in *output
void getcmd(const Block *block, char *output) {
  FILE *cmdf = popen(block->command, "r");
  if (!cmdf)
    return;
  fgets(output, MAXCMDLENGTH, cmdf);
  int i = strlen(output);
  if (i == 0) {
    // return if block and command output are both empty
    pclose(cmdf);
    return;
  }
  // only chop off newline if one is present at the end
  i = output[i - 1] == '\n' ? i - 1 : i;
  output[i++] = '\0';
  pclose(cmdf);
}

void getcmds(int time) {
  const Block *current;
  for (unsigned int i = 0; i < block_num; i++) {
    current = blocks + i;
    if ((current->interval != 0 && time % current->interval == 0) ||
        time == -1) {
      getcmd(current, statusbar[i]);
    }
  }
}

void getsigcmds(unsigned int signal) {
  const Block *current;
  for (unsigned int i = 0; i < block_num; i++) {
    current = blocks + i;
    if (current->signal == signal) {
      if (i == 0)
        strcpy(statusbar[i], delims[i]);
      getcmd(current, statusbar[i]);
    }
  }
}

void setupsignals() {
#ifndef __OpenBSD__
  /* initialize all real time signals with dummy handler */
  for (int i = SIGRTMIN; i <= SIGRTMAX; i++)
    signal(i, dummysighandler);
#endif

  for (unsigned int i = 0; i < block_num; i++) {
    if (blocks[i].signal > 0)
      signal(SIGMINUS + blocks[i].signal, sighandler);
  }
}

int getstatus(char *str, char *last) {
  strcpy(last, str);
  str = strcpy(str, delims[0]);
  for (unsigned int i = 0; i < block_num; i++) {
    strcat(str, statusbar[i]);
    strcat(str, delims[i+1]);
  }
  str[strlen(str)] = '\0';
  return strcmp(str, last); // 0 if they are the same
}

void writestatus() {
  // Only write out if text has changed.
  if (!getstatus(statusstrnew, statusstrold))
    return;
  printf("%s\n", statusstrnew);
  fflush(stdout);
}

void statusloop() {
  setupsignals();
  int i = 0;
  getcmds(-1);
  while (1) {
    getcmds(i++);
    writestatus();
    if (!statusContinue)
      break;
    sleep(1.0);
  }
}

#ifndef __OpenBSD__
/* this signal handler should do nothing */
void dummysighandler(int signum) { return; }
#endif

void sighandler(int signum) {
  getsigcmds(signum - SIGPLUS);
  writestatus();
}

void termhandler() { statusContinue = 0; }

int main(int argc, char **argv) {
  // Handle command line arguments
  static char configfile[100];
  if (strlen(argv[1]) > 0)
    strcpy(configfile+0, argv[1]);
  parseconfig(configfile);
  signal(SIGTERM, termhandler);
  signal(SIGINT, termhandler);
  statusloop();
  free_memory();
  return 0;
}
