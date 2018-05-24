#include "mpi.h"
#include "definitions.h"

/** Declarations of changing rooms */
int m[3];
int w[3];
int changingRoom = 0;
int received = 0;
int gender;

int requestingChangingRoom = 0;
int requestsWHP = 0;
int requestsWHPAG = 0;
int doIrequestChangingRoom = 0;
int reqeusts[1000]; //TODO zmienic 1000 na inna liczbe

int size = 0;
int ending = 0;
int ended = 0;
int processID = 0;
int currentHash = 0;

int sendingMessage[MSG_SIZE]; // 0 - , 1 -
int receiveMessage[MSG_SIZE]; // 0 - , 1 -
int replyMessage[MSG_SIZE]; // 0 - , 1 -
MPI_Status status;


void setUpRand(int pid){
	time_t tt;
    srand(time(&tt)*pid);
}

/** Set occupancy of all changing rooms to 0 */
void reset(){ 
	currentHash = 0;
	changingRoom = 0;
	requestingChangingRoom = requestsWHP = requestsWHPAG = 0;
	received = 1; //od siebie juz dostalismy wiadomosc jakby
	m[0] = m[1] = m[2] = 0;//1-2-3
	w[0] = w[1] = w[2] = 0;//4-5-6
}

/** Return gender of client. Returns 1 - for men, 2 - for women */
int getGender(){
	return rand()%2;
}

char *getGenderStr(int g){
	return (g==0)?KCYNB"M"KGRN:KMAGB"W"KGRN;
}

void *loop(void *ptr){
	//int *myid = (int *)ptr;
	int receivedVals;
	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = 10000L;
	while(ending == 0){
		MPI_Recv(receiveMessage, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		MPI_Get_count(&status, MPI_INT, &receivedVals);
		//printf("%d received from %d, tag %d\n",processID, status.MPI_SOURCE, status.MPI_TAG);
		//printf("%d receive from %d [%d, %d, %s]\n", processID, status.MPI_SOURCE, receiveMessage[0], receiveMessage[1],(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST");
		if(status.MPI_TAG == MSG_TAG_REQUEST) {
			if(doIrequestChangingRoom == 1){
				++requestingChangingRoom;
				if(receiveMessage[MSG_HASH] < currentHash) {
					++requestsWHP;
					if(receiveMessage[MSG_GENDER] != gender) ++requestsWHPAG;
				}
			}
			
			replyMessage[MSG_HASH] = receiveMessage[MSG_HASH];
			replyMessage[MSG_ROOM] = changingRoom;
			//printf("%d is replying [%d, %d] to %d\n",processID, replyMessage[MSG_ROOM],replyMessage[MSG_HASH],status.MPI_SOURCE);
			//printf("status: tag %d, source: %d\n",status.MPI_TAG, status.MPI_SOURCE);
			//int sendTo = status.MPI_SOURCE;
			MPI_Send(replyMessage, MSG_SIZE, MPI_INT,status.MPI_SOURCE, MSG_TAG_REPLY, MPI_COMM_WORLD);
			/*printf(KGRN"%d receive from [%d, %s, room %d]\t Sent: [%d] to %d  \t%s %010d\n"KNRM, 
			processID, 
			status.MPI_SOURCE, 
			getGenderStr(receiveMessage[MSG_GENDER]),
			receiveMessage[MSG_ROOM],
			replyMessage[MSG_ROOM],
			status.MPI_SOURCE,
			(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST",
			receiveMessage[MSG_HASH]);*/
		} else if(receiveMessage[MSG_HASH] == currentHash){
			//printf("%d received from %d, room: %d\n",processID, status.MPI_SOURCE, receiveMessage[MSG_ROOM]);
			received++;
			if(receiveMessage[MSG_ROOM]>3) w[receiveMessage[MSG_ROOM]-4]++;
			if(receiveMessage[MSG_ROOM]>0) m[receiveMessage[MSG_ROOM]-1]++;
		}
		//printf("%d receive from %d [%d, %d, %s]", processID, status.MPI_SOURCE, receiveMessage[0], receiveMessage[1],(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST");
		//if(status.MPI_TAG == MSG_TAG_REQUEST) printf("\t Sent: [%d, %d] to %d\n",replyMessage[MSG_HASH],replyMessage[MSG_ROOM],status.MPI_SOURCE);
		//else printf("\n");
		
		nanosleep(&tim, NULL);
	}
	ended = 1;
	return NULL;
}
/** Checks if process can occupy place in changing room
 * Arguments: r1, r2 - number of men/women or women/men in specified chaning room
 * return: 	0 - current process can't occupy place in specified chaning room
 * 			1 - current process can occupy place in specified chaning room
 */
int canJoinRoom(int r1, int r2){
	if(r1==0 && (r2 > 0 || r2 + requestsWHPAG > 0)) {/*printf(KGRN"' "KNRM);*/return 0;}
	if(size == received && MAX_IN_ROOM - r1 - (requestsWHPAG - requestsWHP) > 0) {/*printf(KRED"' "KNRM);*/return 1;}
	if(MAX_IN_ROOM - r1 - (size - received) - (requestsWHPAG - requestsWHP) > 0) {/*printf(KBLU"' "KNRM);*/return 1;}
	
	//sprawdzenie czy inni nie chca joinowac :/
	return 0;
}



int waitAndJoinChangingRoom(){
	while(changingRoom == 0 && received != size){
		if(gender == 0){
			if(canJoinRoom(m[0],w[0])==1)changingRoom = 1;
			if(canJoinRoom(m[1],w[1])==1)changingRoom = 2;
			if(canJoinRoom(m[2],w[2])==1)changingRoom = 3;
		}else{
			if(canJoinRoom(w[0],m[0])==1)changingRoom = 4;
			if(canJoinRoom(w[1],m[1])==1)changingRoom = 5;
			if(canJoinRoom(w[2],m[2])==1)changingRoom = 6;
		}
	}
	if(received == size){
		//zresetiowac i wysalc ponownie zadanie
		return doIrequestChangingRoom;
	}
	else{
		printf(KRED"OCCUPY CHANING ROOM "KYELB"ID: %d\tRoom: %d(%d)\tWho: %s\t\t"KNRM""KYELB"["KCYNB"%d"KYELB",%d"KYELB",%s"KYELB"]\n",
		processID,
		(changingRoom-1)%3+1,
		changingRoom,
		getGenderStr(gender),
		
		processID,(changingRoom-1)%3+1,getGenderStr(gender)
		);
		//doIrequestChangingRoom = 0;
		return doIrequestChangingRoom = 0;
	}
}

int generateHash(int timesHB){
	timesHB++;
	int result = (timesHB*128 + processID) * 1024;
	result = result + rand()%1024;
	return result;
}

void *sender(void *ptr){
	int howManyTimesGoIntoThermal = rand()%5+1;//1-6
	printf(KYEL"New process "KRED"%d"KYEL" %s"KYEL" will go "KRED"%d"KYEL" times to thermal\n"KNRM,processID,getGenderStr(gender), howManyTimesGoIntoThermal);
	
	struct timespec tim;
	tim.tv_sec = rand()%10+5; //every 5-15 sec request joining to chaning rooms
	//tim.tv_nsec = 1000L;
	int i;
	nanosleep(&tim, NULL);
	int timesHaveBeen = 0;
	while(1){//timesHaveBeen++ < howManyTimesGoIntoThermal
		doIrequestChangingRoom = 1;
		reset();
		sendingMessage[MSG_GENDER] = gender;
		sendingMessage[MSG_ROOM] = 0;
		sendingMessage[MSG_HASH] = currentHash = generateHash(timesHaveBeen);
		for(i=0;i<size;++i){
			if (i != processID){
				//printf("%d is sending [%d, %d] to %d\n",processID,sendingMessage[MSG_GENDER],sendingMessage[MSG_HASH], i);
				MPI_Send(sendingMessage, MSG_SIZE, MPI_INT, i, MSG_TAG_REQUEST, MPI_COMM_WORLD);
			}
		}
		//MPI_Bcast(sendingMessage,MSG_SIZE,MPI_INT, processID,MPI_COMM_WORLD);
		
		if(waitAndJoinChangingRoom()==0){
			timesHaveBeen++;
			tim.tv_sec = rand()%10+10;
			nanosleep(&tim, NULL);
			printf("%d frees space in %d\n",processID,changingRoom);
		}else{
			tim.tv_sec = 4;
			nanosleep(&tim, NULL);
		}
	}
	ending = 1;
	return NULL;
}

void testRandom(){
	if(processID != 0) return;
	int j = 0;
	for(j = 0;j<5;++j){
		printf(KGRN"%010d\n",generateHash(j));
		printf(KBLU"%010d\n",generateHash(j));
		printf(KCYN"%010d\n",generateHash(j));
	}
}

int main(int argc, char **argv)
{
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &processID );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	setUpRand(processID);
	//testRandom();
	gender = getGender();
	//printf("New process %d %s\n",processID,(gender==0)?"man":"woman");
	//hello world
	pthread_t th;
	pthread_create(&th, NULL,loop,NULL);
	pthread_create(&th, NULL,sender,NULL);

	while(ended != 1);
	MPI_Finalize();
}
