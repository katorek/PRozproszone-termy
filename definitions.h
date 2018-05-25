#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define MSG_SIZE 3
#define MSG_TAG_REQUEST 100
#define MSG_TAG_REPLY 101

#define MSG_GENDER 2
#define MSG_HASH 1
#define MSG_ROOM 0

#define MAX_IN_ROOM 2

#define KNRM   "\x1B[0m"
#define KRED   "\x1B[31m"
#define KREDB  "\x1B[91m"
#define RED  "\x1B[91m"
#define KGRN   "\x1B[32m"
#define KGRNB  "\x1B[92m"
#define GR  "\x1B[92m"
#define KYEL   "\x1B[33m"
#define KYELB  "\x1B[93m"
#define Y  "\x1B[93m"
#define KBLU   "\x1B[34m"
#define KBLUB  "\x1B[94m"
#define BL  "\x1B[94m"
#define KMAG   "\x1B[35m"
#define KMAGB  "\x1B[95m"
#define KCYN   "\x1B[36m"
#define KCYNB  "\x1B[96m"
#define C  "\x1B[96m"
#define KWHT   "\x1B[37m"
