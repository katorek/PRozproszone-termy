#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MSG_SIZE 3
#define MSG_TAG_REQUEST 100
#define MSG_TAG_REPLY 101

#define MSG_GENDER 2
#define MSG_HASH 1
#define MSG_ROOM 0

#define MAX_IN_ROOM 2

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KGRNB  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KYELB  "\x1B[93m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KMAGB  "\x1B[95m"
#define KCYN  "\x1B[36m"
#define KCYNB  "\x1B[96m"
#define KWHT  "\x1B[37m"
