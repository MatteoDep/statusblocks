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
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define MAXCMDLENGTH 1000
#define MAXCFGLINELENGTH 500

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
void pstdout();
static void (*writestatus)() = pstdout;

typedef struct {
  char *command;
  unsigned int interval;
  unsigned int signal;
} Block;

static Block blocks[] = {};
static char *delims[] = {};

static char *statusbar[MAXCMDLENGTH] = {};
static char *statusstr[2];
static int statusContinue = 1;
static int returnStatus = 0;
static char *configfile;

void trim(char *line) {
  // strips away leading and trailing spaces
  int start = 0;
  int len = strlen(line);
  int end = len;
  unsigned int count = 0;
  while ((start < end) && (count < 2)) {
    if ((line[start] != ' ') && (line[start] != '\t'))
      count++;
    else
      start++;
    if ((line[end] != ' ') && (line[end] != '\t') && (line[end] != '\n'))
      count++;
    else
      end--;
  }
  strncpy(line, line + start, end - start);
}

void cfgline2block(Block block, char *line) {
  char *interval;
  char *signal;
  char *command;
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
          strcpy(command, line + start);
        isprevspace = 0;
      }
    }
  }
}

void parseconfig() {
  FILE *config = fopen(configfile, "r");
  if (!config) {
    printf("config file %s not found.\n", configfile);
    exit(EXIT_FAILURE);
  }

  char line[MAXCFGLINELENGTH];
  int i = 0;
  *(delims + i) = "";
  while (fgets(line, MAXCFGLINELENGTH, config)) {
    trim(line);
    switch (line[0]) {
    case '\0':
      break;
    case '#':
      break;
    case '\"':
      strncpy(*(delims + i), line + 1, strlen(line) - 2);
      break;
    default:
      cfgline2block(*(blocks + 1), line);
      i++;
      *(delims + i) = "";
    }
  }
  fclose(config);
}

// opens process *cmd and stores output in *output
void getcmd(const Block *block, char *delim, char *output) {
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
  if (delim[0] != '\0') {
    // only chop off newline if one is present at the end
    i = output[i - 1] == '\n' ? i - 1 : i;
    strcpy(output + i, delim);
  } else
    output[i++] = '\0';
  pclose(cmdf);
}

void getcmds(int time) {
  const Block *current;
  for (unsigned int i = 0; i < LENGTH(blocks); i++) {
    current = blocks + i;
    if ((current->interval != 0 && time % current->interval == 0) ||
        time == -1) {
      if (i == 0)
        strcpy(statusbar[i], delims[i]);
      getcmd(current, delims[i + 1], statusbar[i]);
    }
  }
}

void getsigcmds(unsigned int signal) {
  const Block *current;
  for (unsigned int i = 0; i < LENGTH(blocks); i++) {
    current = blocks + i;
    if (current->signal == signal) {
      if (i == 0)
        strcpy(statusbar[i], delims[i]);
      getcmd(current, delims[i + 1], statusbar[i]);
    }
  }
}

void setupsignals() {
#ifndef __OpenBSD__
  /* initialize all real time signals with dummy handler */
  for (int i = SIGRTMIN; i <= SIGRTMAX; i++)
    signal(i, dummysighandler);
#endif

  for (unsigned int i = 0; i < LENGTH(blocks); i++) {
    if (blocks[i].signal > 0)
      signal(SIGMINUS + blocks[i].signal, sighandler);
  }
}

int getstatus(char *str, char *last) {
  strcpy(last, str);
  str[0] = '\0';
  for (unsigned int i = 0; i < LENGTH(blocks); i++)
    strcat(str, statusbar[i]);
  str[strlen(str)] = '\0';
  return strcmp(str, last); // 0 if they are the same
}

void pstdout() {
  // Only write out if text has changed.
  if (!getstatus(statusstr[0], statusstr[1]))
    return;
  printf("%s\n", statusstr[0]);
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
  if (!strlen(argv[1]))
    strcpy(configfile, argv[1]);
  parseconfig();
  signal(SIGTERM, termhandler);
  signal(SIGINT, termhandler);
  signal(SIGUSR1, parseconfig);
  statusloop();
  return 0;
}
