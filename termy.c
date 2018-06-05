#include "mpi.h"
#include "definitions.h"

/** Declarations of changing rooms */
int m[3];
int w[3];
int changingRoom = 0;
int received = 0;
int gender;

int requestingChangingRoom = 0;
int requestsWHPDiffrentG = 0;
int requestsWHPSameG = 0;
int doIrequestChangingRoom = 0;

int maxInRoom = 0;
int size = 0;
int ending = 0;
int ended = 0;
int processID = 0;
int currentHash = 0;
int howManySpaces = 2;
int multiplier = 1;

int sendingMessage[MSG_SIZE];
int receiveMessage[MSG_SIZE];
int replyMessage[MSG_SIZE];
MPI_Status status;


pthread_mutex_t lock;

/** Sets seed for random */
void setUpRand(int pid){
	time_t tt;
    srand(time(&tt)*pid);
}

/** Set occupancy of all changing rooms to 0. Also resets required variables to default values */
void reset(){ 
	pthread_mutex_lock(&lock);
	//currentHash = 0;
	requestingChangingRoom = requestsWHPDiffrentG = requestsWHPSameG = 0;
	changingRoom = 0;
	received = 1; //od siebie juz dostalismy wiadomosc
	m[0] = m[1] = m[2] = 0;//1-2-3
	w[0] = w[1] = w[2] = 0;//4-5-6
	pthread_mutex_unlock(&lock);
}

/** Return gender of client. Returns 0 - for men, 1 - for women */
int getGender(){
	return rand()%2;
}

char *getGenderStr(int g){
	return (g==0)?C"M"GR:M"W"GR;
}

int printTables(){
	int c;
	c = m[0];
	c = c*10 + m[1];
	c = c*10 + m[2];
	c = c*10 + w[0];
	c = c*10 + w[1];
	c = c*10 + w[2];
	return c;
}

/** Endless loop, which is responsible for receiving messages and handling them properly */
void *loop(void *ptr){
	int receivedVals;
	while(ending == 0){
		MPI_Recv(receiveMessage, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_INT, &receivedVals);
		
		if(status.MPI_TAG == MSG_TAG_REQUEST) {
			if(doIrequestChangingRoom == 1){
				++requestingChangingRoom;
				if(receiveMessage[MSG_HASH] < currentHash || (receiveMessage[MSG_HASH] == currentHash && status.MPI_SOURCE < processID)) {
					if(receiveMessage[MSG_GENDER] != gender) requestsWHPDiffrentG++;
					else requestsWHPSameG++;
				}
			}
			
			replyMessage[MSG_HASH] = receiveMessage[MSG_HASH];
			replyMessage[MSG_ROOM] = changingRoom;
			MPI_Send(replyMessage, MSG_SIZE, MPI_INT,status.MPI_SOURCE, MSG_TAG_REPLY, MPI_COMM_WORLD);
		} else if(status.MPI_TAG == MSG_TAG_REPLY && receiveMessage[MSG_HASH] == currentHash){
			pthread_mutex_lock(&lock);
			received++;
			if(receiveMessage[MSG_ROOM]>3) w[receiveMessage[MSG_ROOM]-4]++;
			else if(receiveMessage[MSG_ROOM]>0) m[receiveMessage[MSG_ROOM]-1]++;
			pthread_mutex_unlock(&lock);
			
		} else if(status.MPI_TAG == MSG_TAG_ENTERS && doIrequestChangingRoom == 1){
			pthread_mutex_lock(&lock);
			
			if(receiveMessage[MSG_ROOM]>3) w[receiveMessage[MSG_ROOM]-4]++;// = (w[receiveMessage[MSG_ROOM]-4]>0)?w[receiveMessage[MSG_ROOM]-4]+1:0;
			else if(receiveMessage[MSG_ROOM]>0) m[receiveMessage[MSG_ROOM]-1]++;// = (m[receiveMessage[MSG_ROOM]-1]>0)?m[receiveMessage[MSG_ROOM]-1]+1:0;
			
			//if(doIrequestChangingRoom == 1){
				if(receiveMessage[MSG_HASH] < currentHash || (receiveMessage[MSG_HASH] == currentHash && status.MPI_SOURCE < processID)) {
					//if(receiveMessage[MSG_GENDER] != gender) requestsWHPDiffrentG=(requestsWHPDiffrentG>0)?requestsWHPDiffrentG-1:0;
					//else requestsWHPSameG=(requestsWHPSameG>0)?requestsWHPSameG-1:0;
					
					if(receiveMessage[MSG_GENDER] != gender) requestsWHPDiffrentG--;//=(requestsWHPDiffrentG>0)?requestsWHPDiffrentG-1:0;
					else requestsWHPSameG--;//=(requestsWHPSameG>0)?requestsWHPSameG-1:0;
				}
			//}
			
			pthread_mutex_unlock(&lock);
			
		} else if(status.MPI_TAG == MSG_TAG_LEAVES && doIrequestChangingRoom == 1){
			pthread_mutex_lock(&lock);
			if(receiveMessage[MSG_ROOM]>3) w[receiveMessage[MSG_ROOM]-4]--;// = (w[receiveMessage[MSG_ROOM]-4]>0)?w[receiveMessage[MSG_ROOM]-4]-1:0;
			else if(receiveMessage[MSG_ROOM]>0) m[receiveMessage[MSG_ROOM]-1]--;// = (m[receiveMessage[MSG_ROOM]-1]>0)?m[receiveMessage[MSG_ROOM]-1]-1:0;
			/*
			if(doIrequestChangingRoom == 1){
			
				if(receiveMessage[MSG_HASH] < currentHash || (receiveMessage[MSG_HASH] == currentHash && status.MPI_SOURCE < processID)) {
					if(receiveMessage[MSG_GENDER] != gender) requestsWHPDiffrentG=(requestsWHPDiffrentG>0)?requestsWHPDiffrentG-1:0;
					else requestsWHPSameG=(requestsWHPSameG>0)?requestsWHPSameG-1:0;
				}
			}*/
			pthread_mutex_unlock(&lock);
		}
	}
	ended = 1;
	printf(RED"ENDING LOOP"C"%d",processID);
	return NULL;
}

/** Checks if process can occupy place in changing room
 * Args: r1, r2 - number of men/women or women/men in specified changing room
 * return: 	0 - current process can't occupy place in specified changing room
 * 			1 - current process can occupy place in specified changing room
 */
int canJoinRoom(int r1, int r2){
	if(r1==0 && (r2 > 0 || r2 + requestsWHPDiffrentG > 0)) {return 0;}
	if(r2==0 && size == received && ((maxInRoom - r1 - (requestsWHPSameG)) > 0)) {printf(GR"");return 1;}
	if(r2==0 && r1>0 && (maxInRoom - r1 - (size - received) - (requestsWHPSameG)) > 0) {printf(BL"");return 1;}
	return 0;
}

/** Loop in which process checks if can join any changing room
 *	Returns:	0 - process found changing room
 *				1 - process was unable to find suitable changing room 
 */
int waitAndJoinChangingRoom(){
	while(changingRoom == 0){
		if(gender == 0){
			pthread_mutex_lock(&lock);
			if		(canJoinRoom(m[0], w[0]) == 1) changingRoom = 1;
			else if	(canJoinRoom(m[1], w[1]) == 1) changingRoom = 2;
			else if	(canJoinRoom(m[2], w[2]) == 1) changingRoom = 3;
			pthread_mutex_unlock(&lock);
		}else{
			pthread_mutex_lock(&lock);
			if		(canJoinRoom(w[0], m[0]) == 1) changingRoom = 4;
			else if	(canJoinRoom(w[1], m[1]) == 1) changingRoom = 5;
			else if	(canJoinRoom(w[2], m[2]) == 1) changingRoom = 6;
			pthread_mutex_unlock(&lock);
		}
		//if(received == size) break;
	}
	//if(changingRoom==0 && received == size){
	//	return doIrequestChangingRoom;
	//}
	//else{
	printf("ENTERS\t"Y"%d <-"GR"%*d%s"Y" | M["C"%03d"Y"]  W["C"%03d"Y"]\tR[HPriorSameGen,HPOtherGen,ReceivedReply]: "C"%d"Y"["C"%d"Y","C"%d"Y","C"%d"Y"]\t"Y"%d\n",
		(changingRoom-1)%3+1,
		howManySpaces,
		processID,
		getGenderStr(gender),
		printTables()/1000,
		printTables()%1000,
		requestingChangingRoom,
		requestsWHPSameG,
		requestsWHPDiffrentG,
		received,
		currentHash
	);
	return doIrequestChangingRoom = 0;
	//}
}
/*
	printf(Y"WANT JOIN: "C"%d\n"KNRM, processID);
*/
/** Generating random hash/priority which depends on how many times current process was in changing room */
int generateHash(int timesHB){
	return (++timesHB*multiplier + (processID+1))*multiplier + rand()%(multiplier/2);
}

int showWantToJoin = 0;

void *sender(void *ptr){
	printf(Y"New process "GR"%d"Y" %s"KNRM"\n",
		processID,
		getGenderStr(gender));
	
	struct timespec tim;
	tim.tv_sec = rand()%(MAX_WAIT_TIME-MIN_WAIT_TIME)+MIN_WAIT_TIME; //every 10-20 sec request joining to changing rooms
	//tim.tv_nsec = 1000L;
	int i;
	nanosleep(&tim, NULL);
	int timesHaveBeen = 0;
	while(1){
		if(showWantToJoin==1)printf(M"REQUEST "C"%d\n"KNRM, processID);
		doIrequestChangingRoom = 1;
		reset();
		sendingMessage[MSG_GENDER] = gender;
		sendingMessage[MSG_ROOM] = 0;
		sendingMessage[MSG_HASH] = currentHash = generateHash(timesHaveBeen);
		
		
		
		
		for(i=0;i<size;++i){
			if (i != processID){
				MPI_Send(sendingMessage, MSG_SIZE, MPI_INT, i, MSG_TAG_REQUEST, MPI_COMM_WORLD);
			}
		}
		
		if(waitAndJoinChangingRoom()==0){
			sendingMessage[MSG_GENDER] = gender;
			sendingMessage[MSG_ROOM] = changingRoom;
			sendingMessage[MSG_HASH] = currentHash;
			
			
			//wyslac do wszystkich ze wszedlem do szatni changingRoom
			for(i=0;i<size;++i){
				if (i != processID){
					MPI_Send(sendingMessage, MSG_SIZE, MPI_INT, i, MSG_TAG_ENTERS, MPI_COMM_WORLD);
				}
			}
			
			
			
			timesHaveBeen++;
			tim.tv_sec = rand()%(MAX_WAIT_TIME-MIN_WAIT_TIME)+MIN_WAIT_TIME;
			nanosleep(&tim, NULL);
			//wychodizmy z term i informujemy innych o tym
			sendingMessage[MSG_ROOM] = changingRoom;
			sendingMessage[MSG_HASH] = currentHash;
			sendingMessage[MSG_GENDER] = gender;
			
			for(i=0;i<size;++i){
				if (i != processID){
					MPI_Send(sendingMessage, MSG_SIZE, MPI_INT, i, MSG_TAG_LEAVES, MPI_COMM_WORLD);
				}
			}
			
			printf(RED"LEAVES\t"Y"%d,"C"%*d"Y"\n",
				(changingRoom-1)%3+1,
				howManySpaces,
				processID
				);
			changingRoom = 0;
			tim.tv_sec = rand()%(MAX_WAIT_TIME-MIN_WAIT_TIME)+MIN_WAIT_TIME*2;
			nanosleep(&tim, NULL);
		}else{
			//doIrequestChangingRoom = 2;
			tim.tv_sec = rand()%RETRY_TIME;
			nanosleep(&tim, NULL);
		}
	}
	printf(RED"ENDING SENDER"C"%d",processID);
	ending = 1;
	return NULL;
}

int main(int argc, char **argv)
{
	maxInRoom = MAX_IN_ROOM;
	if(argc > 1) {
		maxInRoom = atoi(argv[1]);
	}
	pthread_mutex_init(&lock, NULL);
	MPI_Init(&argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	howManySpaces = (int) log10((double) size - 1) + 1;
	MPI_Comm_rank( MPI_COMM_WORLD, &processID );
	if(processID==0)
		printf(RED"SIZE: "C"%d "RED"MAX IN ROOM: "C"%d"RED" LOG: "C"%d\n",
			size, 
			maxInRoom,
			howManySpaces
		);
	int j;
	for(j=0;j<howManySpaces;++j){
		multiplier=multiplier*10;
	}
	setUpRand(processID);
	gender = getGender();

	pthread_t th;
	pthread_create(&th, NULL,loop,NULL);
	pthread_create(&th, NULL,sender,NULL);

	struct timespec tim2;
	tim2.tv_sec = 1;
	//tim2.tv_nsec = 10000L;
	while(ended != 1) nanosleep(&tim2,NULL);
	MPI_Finalize();
}
