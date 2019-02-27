#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SZ 128
#define BUF_SZZ 100000

/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */

/**
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)
*/

/* Function prototypes */
void parse_sys();
void print_sysinfo(char *name, char *version, char *uptimeVal);
void parse_Uptime(char *uptime);
void parse_hardware();
float get_cpuUsage();
void print_hardwareinfo(char *model, int units, char *avg1, char *avg2, char *avg3, float cpu, float memory, float gb1, float gb2);
void parse_task();
void print_taskinfo(int tasks, char *inter, char *switches, char *forks);
void get_taskList();
void print_taskList(char *pid, char *state, char *taskName, char *user, int tasks);
void print_usage(char *argv[]);
char *next_token(char **str_ptr, const char *delim);


/* This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled. */
struct view_opts {
    bool hardware;
    bool system;
    bool task_list;
    bool task_summary;
};

/* Functions */
void print_usage(char *argv[]) {
    printf("Usage: %s [-ahlrst] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -lrst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Task List\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}
char *next_token(char **str_ptr, const char *delim) {
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }
    return current_ptr;
}

/* System Information */

/**
	* Parses files to obtain hostname, kernel version, and uptime
	*/
void parse_sys() {
  /*GETS HOSTNAME*/
  //opens file
  int fd = open("sys/kernel/hostname", O_RDONLY);
  //checks for invalid opening
  if (fd == -1) {
      perror("open");
  }
  //initializes buffer
  char buf[BUF_SZ];
  //reads file
  read(fd, buf, BUF_SZ);
  //mallocs space to read file
  char *fileContent = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent, buf);
  //splits by newline
  char *name = next_token(&fileContent, "\n");
  //closes file
  close(fd);

  /*GETS KERNEL VERSION*/
  //opens file
  int fd2 = open("sys/kernel/osrelease", O_RDONLY);
  //checks for invalid opening
  if (fd2 == -1) {
      perror("open");
  }
  char buf2[BUF_SZ];
  //reads file
  read(fd2, buf2, BUF_SZ);
  //mallocs space to read file
  char *fileContent2 = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent2, buf2);
  //splits by newline
  char *kernelVersion = next_token(&fileContent2, "\n");
  //closes file
  close(fd2);

  /*GETS UPTIME*/
  //opens file
  int fd3 = open("uptime", O_RDONLY);
  //checks for invalid opening
  if (fd3 == -1) {
      perror("open");
  }
  char buf3[BUF_SZ];
  //reads file
  read(fd3, buf3, BUF_SZ);
  //mallocs space to read file
  char *fileContent3 = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent3, buf3);
  //splits by space
  char *uptime = next_token(&fileContent3, " ");
  //closes file
  close(fd3);

  print_sysinfo(name, kernelVersion, uptime);

  //frees next_token
  free(name);
  free(kernelVersion);
  free(uptime);
}

/**
	* Prints data parsed from parse_sys() to console
	*
	* @param name
  * @param version
  * @param uptimeVal
	*/
void print_sysinfo(char *name, char *version, char *uptimeVal) {
    printf("System Information\n");
    printf("------------------\n");
    printf("Hostname: %s\n", name);
    printf("Kernel Version: %s\n", version);
    printf("Uptime: ");
    parse_Uptime(uptimeVal);
    printf("\n");
}

/**
	* Processing of uptime to support years, days, hours, minutes,
  * and seconds
	*
	* @param uptime
	*/
void parse_Uptime(char *uptime) {
  //string to double
  double uptimeVal = atoi(uptime);
  //divides by secs in a year
  double yr = floor(uptimeVal/31536000);
  double yrRemainder = uptimeVal - (yr * 31536000);
  //divides by secs in a day
  double dy = floor(yrRemainder/86400);
  double dyRemainder = yrRemainder - (dy * 86400);
  //divides by secs in an hour
  double hr = floor(dyRemainder/3600);
  double hrRemainder = dyRemainder - (hr * 3600);
  //divides by sec in a min
  double min = floor(hrRemainder/60);
  double minRemainder = hrRemainder - (min * 60);

  if(yr != 0) {
    printf("%d years, ", (int)yr);
  }
  if(dy != 0) {
    printf("%d days, ", (int)dy);
  }
  if(hr != 0) {
    printf("%d hours, ", (int)hr);
  }
  printf("%d minutes, ", (int)min);
  printf("%d seconds\n", (int)minRemainder);
}

/* Hardware Information */

/**
	* Parses files to obtain cpu model name, number of processing units,
  * load averages, cpu usage, and memory usage.
  *
	*/
void parse_hardware() {
  /*GETS CPU MODEL*/
  //opens file
  int fd = open("cpuinfo", O_RDONLY);
  //checks for invalid opening
  if (fd == -1) {
      perror("open");
  }
  //initializes buffer
  char buf[BUF_SZ];
  //reads file
  read(fd, buf, BUF_SZ);
  //mallocs space to read file
  char *fileContent = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent, buf);
  //will store line that contains cpu model name
  char *line = next_token(&fileContent, "\n");
  char *modelName = malloc(50 * sizeof(char));
  //tracks when we got cpu model name
  int modelTracker = 0;
  while(modelTracker < 1) {
    modelName = next_token(&line, " :");
    if(strcmp(modelName, "model") == 0){
      modelName = next_token(&line, " :\t\n");
      if(strcmp(modelName, "name") == 0) {
        modelName = next_token(&line, ":");
        modelTracker++;
      }
    }
    line = next_token(&fileContent, "\n");
  }
  close(fd);


  /*GETS PROC UNITS*/
  //opens file
  int fd1 = open("cpuinfo", O_RDONLY);
  //checks for invalid opening
  if (fd1 == -1) {
      perror("open");
  }
  int numProcUnits = 0;
  //initializes buffer
  char buf1[BUF_SZZ];
  //reads file
  read(fd1, buf1, BUF_SZZ);
  //mallocs space to read file
  char *fileContent1 = malloc(BUF_SZZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent1, buf1);
  //will store line that contains cpu model name
  char *contents1 = malloc(BUF_SZZ * sizeof(char));
  while(contents1 != NULL) {
    contents1 = next_token(&fileContent1, " \n\t:");
    if(contents1 != NULL && strcmp(contents1, "processor") == 0) {
      numProcUnits++;
    }
  }
  close(fd1);

  /*GETS LOAD AVERAGE*/
  //opens file
  int fd2 = open("loadavg", O_RDONLY);
  //checks for invalid opening
  if (fd2 == -1) {
      perror("open");
  }
  char buf2[BUF_SZ];
  //reads file
  read(fd2, buf2, BUF_SZ);
  //mallocs space to read file
  char *fileContent2 = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent2, buf2);
  char *avg1 = next_token(&fileContent2, " ");
  char *avg2 = next_token(&fileContent2, " ");
  char *avg3 = next_token(&fileContent2, " ");
  close(fd2);

  /*GETS CPU USAGE*/
  float cpuUsage = get_cpuUsage();

  /*GETS MEMORY USAGE*/
  //opens file
  int fd3 = open("meminfo", O_RDONLY);
  //checks for invalid opening
  if (fd3 == -1) {
      perror("open");
  }
  char buf3[BUF_SZZ];
  //reads file
  read(fd3, buf3, BUF_SZZ);
  //mallocs space to read file
  char *fileContent3 = malloc(BUF_SZZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent3, buf3);
  //keepr track of the line
  int tracker = 0;
  char *newreader = malloc(150 * sizeof(char));
  //will store memory total
  char *memoryTotal = malloc(150 * sizeof(char));
  //will store active total
  char *activeTotal = malloc(150 * sizeof(char));

  char *reader = next_token(&fileContent3, "\n");
  while(reader != NULL) {
    newreader = next_token(&reader, " ");
     if(strcmp(newreader, "MemTotal:") == 0) {
      newreader = next_token(&reader, " ");
      memoryTotal = newreader;
    }
    if(strcmp(newreader, "Active:") == 0) {
      newreader = next_token(&reader, " ");
      activeTotal = newreader;
    }
    reader = next_token(&fileContent3, "\n");
    tracker++;
  }
  //converets str to float
  float memTotal = atoi(memoryTotal);
  float actTotal = atoi(activeTotal);

  float memGB = memTotal/1000000;
  float actGB = actTotal/1000000;

  float memoryPercent = actGB/memGB;
  //closes file
  close(fd3);

  print_hardwareinfo(modelName, numProcUnits, avg1, avg2, avg3, cpuUsage, memoryPercent, actGB, memGB);
}

/**
	* Processing of cpuUsage. Sleeps for 1 second and repeats.
	*
  *@return cpu usage
	*/
float get_cpuUsage() {

  //values being stored
  int total1 = 0;
  int idle1 = 0;
  int total2 = 0;
  int idle2 = 0;

  //opens file
  int fd5 = open("stat", O_RDONLY);
  //checks for invalid opening
  if (fd5 == -1) {
      perror("open");
  }
  char buf5[BUF_SZ];
  //reads file
  read(fd5, buf5, BUF_SZ);
  //mallocs space to read file
  char *fileContent5 = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent5, buf5);
  //splits by line
  char *cpuInfo= next_token(&fileContent5, "\n");
  //keeps track of cpu numbers
  int number5 = 0;
  //gets the cpuInfo and splits it my space
  char *nums = malloc(15 * sizeof(char));
  while(number5 < 11) {
    nums = next_token(&cpuInfo, " ");
    if(number5 > 0) {
      total1 += atoi(nums);
      if(number5 == 4) {
        idle1 = atoi(nums);
      }
    }
    number5++;
  }
  //closes file
  close(fd5);

  //sleeps for one second
  sleep(1);

  //opens file after sleep
  int fd4 = open("stat", O_RDONLY);
  //checks for invalid opening
  if (fd4 == -1) {
      perror("open");
  }
  char buf4[BUF_SZ];
  //reads file
  read(fd4, buf4, BUF_SZ);
  //mallocs space to read file
  char *fileContent4 = malloc(BUF_SZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent4, buf4);
  //splits by line
  char *cpuInfo4= next_token(&fileContent4, "\n");
  //keeps track of cpu numbers
  int number4 = 0;
  //gets the cpuInfo and splits it my space
  char *nums4 = malloc(15 * sizeof(char));
  while(number4 < 11) {
    nums4 = next_token(&cpuInfo4, " ");
    if(number4 > 0) {
      total2 += atoi(nums4);
      if(number4 == 4) {
        idle2 = atoi(nums4);
      }
    }
    number4++;
  }
  //closes file
  close(fd4);

  float cpuUsage = 0.0;

  int totalDif = (total2 - total1);
  if(totalDif > 0) {
    cpuUsage = 1 - ((idle2 - idle1) / (total2 - total1));
  }
  return cpuUsage;
}

/**
	* Prints and processes hardware information to display onto
	* console
  *
	* @param model cpu model name
  * @param units number of processing units
  * @param avg1 load average figure over 1 minute
  * @param avg2 load average figure over 5 minutes
  * @param avg3 load average figure over 15 minutes
  * @param cpu cpu usage percentage
  * @param memory memory usage
  * @param gb1 memory total in GB
  * @param gb2 active total in GB
	*/
void print_hardwareinfo(char *model, int units, char *avg1, char *avg2, char *avg3, float cpu, float memory, float gb1, float gb2) {
    printf("Hardware Information\n");
    printf("--------------------\n");
    printf("CPU Model: %s\n", model);
    printf("Processing Units: %d\n", units);
    printf("Load Average (1/5/15 min): %s %s %s \n", avg1, avg2, avg3);
    int poundSign = cpu/0.05;
    printf("CPU Usage:    [");
    for(int i = 0; i < poundSign; i++) {
      printf("#");
    }
    for(int j = poundSign; j < 20; j++) {
      printf("-");
    }
    printf("] %.1f%%\n", cpu);
    printf("Memory Usage: [");
    int poundSign2 = memory/0.05;
    for(int i = 0; i < poundSign2; i++) {
      printf("#");
    }
    for(int j = poundSign2; j < 20; j++) {
      printf("-");
    }
    printf("] %.1f%% (%.1f GB / %.1f GB)\n", (memory * 100) , gb1, gb2);
    printf("\n");
}

/* Task Information */

/**
	* Parses files to obtain tasks running, number of interrupts, context
  * switches, and forks since boot.
	*
	* @param uptime
	*/
void parse_task() {
  /*GETS TASKS RUNNING*/
  DIR *dir;
  int tasks = 0;
  struct dirent *entry;
  struct stat buf1;
  if((dir = opendir ("./")) != NULL) {
    printf("dir: %s\n", dir);
    while ((entry = readdir (dir)) != NULL) {
      printf("entry: %s\n", entry);
      stat(entry->d_name, &buf1);
      if (isdigit(entry->d_name[0]) && S_ISDIR(buf1.st_mode)){
        tasks++;
      }
    }
    closedir (dir);
  }

  /*GETS INTERRUPTS, CONTEXT SWITCHES, and FORKS */
  //opens file
  int fd = open("stat", O_RDONLY);
  //checks for invalid opening
  if (fd == -1) {
      perror("open");
  }
  //initializes buffer
  char buf[BUF_SZZ];
  //reads file
  read(fd, buf, BUF_SZZ);
  //mallocs space to read file
  char *fileContent = malloc(BUF_SZZ * sizeof(char));
  //copies contents over to buffer
  strcpy(fileContent, buf);
  char *finalNum= malloc(30 * sizeof(char));
  char *finalNum2= malloc(30 * sizeof(char));
  char *finalNum3= malloc(30 * sizeof(char));
  //splits by newline
  char *line = next_token(&fileContent, "\n");
  while(line != NULL) {
    char *next = next_token(&line, " ");
    if(strcmp(next, "intr") == 0) {
      finalNum = next_token(&line, " ");
    }
    if(strcmp(next, "ctxt") == 0) {
      finalNum2 = next_token(&line, " ");
    }
    if(strcmp(next, "processes") == 0) {
      finalNum3 = next_token(&line, " ");
    }
    line = next_token(&fileContent, "\n");
  }
  //closes file
  close(fd);
  print_taskinfo(tasks, finalNum, finalNum2 , finalNum3);
}

/**
	* Prints and processes task information to display onto
	* console
	*
	* @param task
  * @param inter
  * @param switches
  * @param forks
	*/
void print_taskinfo(int tasks, char *inter, char *switches, char *forks) {
    printf("Task Information\n");
    printf("----------------\n");
    printf("Tasks running: %d\n", tasks);
    printf("Since boot:\n");
    printf("    Interrupts: %s\n", inter);
    printf("    Context Switches: %s\n", switches);
    printf("    Forks: %s\n", forks);
    printf("\n");
}

/* Task List */

/**
	* Parses files to obtain PID, STtate, Task Name, User, and Tasks.
	*
	*/
void get_taskList() {
  printf(" %5s | %12s | %25s | %15s | %s \n","PID", "State", "Task Name", "User", "Tasks");
  printf("-------+--------------+---------------------------+-----------------+-------\n");
  DIR *dir;
  struct dirent *ent;
  struct stat buf;
  //opens directory
  if((dir = opendir(".")) != NULL) {
    //looks at subdirectories
    while((ent=readdir(dir)) != NULL) {
      stat(ent->d_name, &buf);
      //checks if are PIDS
      if(isdigit(ent->d_name[0]) && S_ISDIR(buf.st_mode)) {
        //goes to PID directory
        chdir(ent->d_name);
        /*GETS STATE AND NAME*/
        //opens file
        int fd = open("status", O_RDONLY);
        //checks for invalid opening
        if (fd == -1) {
            perror("open");
        }
        //initializes buffer
        char buf[BUF_SZZ];
        //reads file
        read(fd, buf, BUF_SZZ);
        //mallocs space to read file
        char *fileContent = malloc(BUF_SZZ * sizeof(char));
        //copies contents over to buffer
        strcpy(fileContent, buf);
        char *name = malloc(50* sizeof(char));
        char *tempname = malloc(50* sizeof(char));
        char *state = malloc(50 * sizeof(char));
        int target = 0;
        //while(target < 4) {
          //splits by newline
          char *line = next_token(&fileContent, "\n");
        while(line != NULL) {
          //gets first word from the line
          char *spaces = next_token(&line, "\t: ");
          if(strcmp(spaces, "Name") == 0) {
            tempname = next_token(&line, "\t: ");
            strncpy(name, tempname, 25);

          }
          if(strcmp(spaces, "State") == 0) {
            state = next_token(&line, "\t: ");
            state = next_token(&line, "\t:()");
          }
          line = next_token(&fileContent, "\n");
          target++;
        }
        //closes file
        close(fd);

        /*GETS TASKS*/
        DIR *pidDir;
        int tasks = 0;
        struct dirent *pidEnt;
        struct stat buf2;
        if ((pidDir = opendir ("task")) != NULL) {
          while ((pidEnt = readdir (pidDir)) != NULL) {
            stat(pidEnt->d_name, &buf2);
            if (isdigit(pidEnt->d_name[0]) && S_ISDIR(buf2.st_mode)) {
              tasks++;
            }
          }
          closedir(pidDir);
        }

        /*GETS USER*/
        //exits PID directory
        chdir("..");

        struct stat buf3;
        struct passwd *word;
        char *usrName = malloc(50 * sizeof(char));
        char *str = malloc(50 * sizeof(char));

        //concatenates PID and passes to stat
        strcpy(str, "./");
        strcat(str, ent->d_name);
        stat(str, &buf3);
        //gets user
        word = getpwuid(buf3.st_uid);
        strcpy(usrName, word->pw_name);

        print_taskList(ent->d_name, state, name, usrName, tasks);
      }
    }
  }
}

/**
	* Prints and processes task list information to display onto
	* console
	*
	* @param pid
  * @param state
  * @param taskName
  * @param user
  * @param tasks
	*/
void print_taskList(char *pid, char *state, char *taskName, char *user, int tasks){
  printf(" %5s | %12s | %25s | %15s | %d \n",pid, state, taskName, user, tasks);
}

/**
  * Parses command line arguments to execute Unix utility
  * @param args
	*/
int main(int argc, char *argv[]) {
    /* Default location of the proc file system */
    char *procfs_loc = "/proc";

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    struct view_opts all_on = { true, true, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1) {
        switch (c) {
            case 'a':
                options = all_on;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 'l':
                options.task_list = true;
                break;
            case 'p':
                procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'r':
                options.hardware = true;
                break;
            case 's':
                options.system = true;
                break;
            case 't':
                options.task_summary = true;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    int invalid = chdir(procfs_loc);
    if(invalid == -1) {
      /* Directory does not exist. */
      perror("Directory does not exist.");
      return EXIT_FAILURE;
    }

    if (alt_proc == true) {
        //LOG("Using alternative proc directory: %s\n", procfs_loc);
        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;
    }

    if (argc <= 1) {
        /* No args (or -p only). Enable all options: */
        options = all_on;
    }

    if(options.system == true) {
      parse_sys();
    }
    if(options.hardware == true) {
      parse_hardware();
    }
    if(options.task_summary == true) {
      parse_task();
    }
    if(options.task_list == true) {
      get_taskList();
    }

    // LOG("Options selected: %s%s%s%s\n",
    //         options.hardware ? "hardware " : "",
    //         options.system ? "system " : "",
    //         options.task_list ? "task_list " : "",
    //         options.task_summary ? "task_summary" : "");
    return 0;
}
