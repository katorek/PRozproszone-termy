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

#define NORM   "\x1B[0m"
#define KNRM   "\x1B[0m"
#define RED  "\x1B[91m"
#define GR  "\x1B[92m"
#define Y  "\x1B[93m"
#define BL  "\x1B[94m"
#define M  "\x1B[95m"
#define C  "\x1B[96m"

#define MAX_IN_ROOM 5
#define MIN_WAIT_TIME 10
#define MAX_WAIT_TIME 20
#define RETRY_TIME 4
