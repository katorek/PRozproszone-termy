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

int sendingMessage[MSG_SIZE]; // 0 - , 1 -
int receiveMessage[MSG_SIZE]; // 0 - , 1 -
int replyMessage[MSG_SIZE]; // 0 - , 1 -
MPI_Status status;


pthread_mutex_t lock;

void setUpRand(int pid){
	time_t tt;
    srand(time(&tt)*pid);
}

/** Set occupancy of all changing rooms to 0 */
void reset(){ 
	currentHash = 0;
	requestingChangingRoom = requestsWHP = requestsWHPDiffrentG = requestsWHPSameG = 0;
	changingRoom = 0;
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


int printTables(){
	int c;
	c = m[0];
	c = c*10 + m[1];
	c = c*10 + m[2];
	c = c*10 + w[0];
	c = c*10 + w[1];
	c = c*10 + w[2];
	return c;
	//return 'M'+'['+c1 +','+c2 +','+c3 +']'+'W'+'['+c4 +','+c5+',' +c6+']';
	//return "M["+c1+","+c2+","+c3+"],W["+c4+","+c5+","+c6+"]";
	//return "M["+(m[0]+'0')+"," +				(m[1]+'0')+"," +				(m[2]+'0')+"] W[" +				(w[0]+'0')+", +				(w[1]+'0')+"," +				(w[2]+'0')+"]";
}
int p = 0;//wszystkie wypisz
int q = 0;//reply wypisz
void *loop(void *ptr){
	//int *myid = (int *)ptr;
	int receivedVals;
	struct timespec tim3;
	tim3.tv_sec = 0;
	tim3.tv_nsec = 10000L;
	while(ending == 0){
		MPI_Recv(receiveMessage, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		MPI_Get_count(&status, MPI_INT, &receivedVals);
		if(p==1)printf(KCYNB"%d"KYELB" received from %d, tag %s, msg[%d,%d,%d]\n"KNRM,
		processID, status.MPI_SOURCE, 
		(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST",
		receiveMessage[0],
		receiveMessage[1],
		receiveMessage[2]
		);
		//printf("%d receive from %d [%d, %d, %s]\n", processID, status.MPI_SOURCE, receiveMessage[0], receiveMessage[1],(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST");
		if(status.MPI_TAG == MSG_TAG_REQUEST) {
			if(doIrequestChangingRoom == 1){
				//printf(KYELB"HASH 1:"KCYNB"%d\t"KYELB"2:"KCYNB"%d\n",currentHash, receiveMessage[MSG_HASH]);
				++requestingChangingRoom;
				if(receiveMessage[MSG_HASH] < currentHash) {
					/*printf(KYELB"["KCYNB"%d"KYELB","KMAGB"%d"KYELB"] < ["KCYNB"%d"KYELB","KMAGB"%d"KYELB"]\n"KNRM,
					processID, currentHash,
					status.MPI_SOURCE, receiveMessage[MSG_HASH]
					);*/
					requestsWHP=requestsWHP+1;
					if(receiveMessage[MSG_GENDER] != gender) requestsWHPDiffrentG=requestsWHPDiffrentG+1;
					else requestsWHPSameG = requestsWHPSameG+1;
				}
			}
			
			replyMessage[MSG_HASH] = receiveMessage[MSG_HASH];
			replyMessage[MSG_ROOM] = changingRoom;
			MPI_Send(replyMessage, MSG_SIZE, MPI_INT,status.MPI_SOURCE, MSG_TAG_REPLY, MPI_COMM_WORLD);
			/*printf(KGRN"%d receive from [%d, %s, room %d]\t Sent: [%d] to %d  \t%s %010d\tReceived:%d\n"KNRM, 
			processID, 
			status.MPI_SOURCE, 
			getGenderStr(receiveMessage[MSG_GENDER]),
			receiveMessage[MSG_ROOM],
			replyMessage[MSG_ROOM],
			status.MPI_SOURCE,
			(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST",
			receiveMessage[MSG_HASH],received);*/
		} else if(receiveMessage[MSG_HASH] == currentHash){
			
			pthread_mutex_lock(&lock);
			received = received+1;
			if(receiveMessage[MSG_ROOM]>3) w[receiveMessage[MSG_ROOM]-4]++;
			else if(receiveMessage[MSG_ROOM]>0) m[receiveMessage[MSG_ROOM]-1]++;
			pthread_mutex_unlock(&lock);
			if(q==1)printf(KGRN"%d received from %d, room: %d, received:"KCYNB"%d"KNRM"\n",processID, 
			status.MPI_SOURCE, 
			receiveMessage[MSG_ROOM],
			received
			);
		}
		//printf("%d receive from %d [%d, %d, %s]", processID, status.MPI_SOURCE, receiveMessage[0], receiveMessage[1],(status.MPI_TAG== MSG_TAG_REPLY)?"MSG_TAG_REPLY":"MSG_TAG_REQUEST");
		//if(status.MPI_TAG == MSG_TAG_REQUEST) printf("\t Sent: [%d, %d] to %d\n",replyMessage[MSG_HASH],replyMessage[MSG_ROOM],status.MPI_SOURCE);
		//else printf("\n");
		
		nanosleep(&tim3, NULL);
	}
	ended = 1;
	printf(KRED"ENDING LOOP"KCYNB"%d",processID);
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
	
	//sprawdzenie czy inni nie chca joinowac :/
	return 0;
}



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
		if(received == size) break;
	}
	if(changingRoom==0 && received == size){
		//printf(KCYNB"%d"KGRN" waits\n",processID);
		//zresetiowac i wysalc ponownie zadanie
		return doIrequestChangingRoom;
	}
	else{
		printf("ENTERS\t"Y"%d <-"GR"%*d%s"Y" | M["C"%03d"Y"]  W["C"%03d"Y"]\tR[HPSameG,HPOtherG,All,ReceivedReply]: "C"%d"Y"["C"%d"Y","C"%d"Y","C"%d,%d]\t"Y"%d\n",
			(changingRoom-1)%3+1,
			howManySpaces,
			processID,
			getGenderStr(gender),
			printTables()/1000,
			printTables()%1000,
			requestsWHP,
			requestsWHPDiffrentG,
			requestsWHPSameG,
			received,
			requestingChangingRoom,
			currentHash
		);
		//doIrequestChangingRoom = 0;
		return doIrequestChangingRoom = 0;
	}
}
int multiplier = 1;
int generateHash(int timesHB){
	return (++timesHB*multiplier + processID)*multiplier + rand()%(multiplier/2);
}

void *sender(void *ptr){
	//int howManyTimesGoIntoThermal = rand()%5+1;//1-6
	printf(Y"New process "GR"%d"Y" %s\n"KNRM,
		processID,
		getGenderStr(gender));
	
	struct timespec tim;
	tim.tv_sec = rand()%10+7; //every 5-15 sec request joining to chaning rooms
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
		/*printf(KRED"Sending requests "KYEL"["KCYNB"%d"KYEL",%s"KYEL"]\n",
			processID, 
			getGenderStr(gender));*/
		for(i=0;i<size;++i){
			if (i != processID){
				//printf("%d is sending [%d, %d] to %d\n",processID,sendingMessage[MSG_GENDER],sendingMessage[MSG_HASH], i);
				MPI_Send(sendingMessage, MSG_SIZE, MPI_INT, i, MSG_TAG_REQUEST, MPI_COMM_WORLD);
			}
		}
		//MPI_Bcast(sendingMessage,MSG_SIZE,MPI_INT, processID,MPI_COMM_WORLD);
		
		if(waitAndJoinChangingRoom()==0){
			timesHaveBeen++;
			tim.tv_sec = rand()%10+15;
			nanosleep(&tim, NULL);//jest na termach
			printf(RED"LEAVES\t"Y"%d,"C"%*d"Y"\n",
				(changingRoom-1)%3+1,
				howManySpaces,
				processID
				);
			changingRoom = 0;
			tim.tv_sec = rand()%10+15;
			nanosleep(&tim, NULL);//jest na termach
		}else{
			tim.tv_sec = rand()%4+2;
			nanosleep(&tim, NULL);
		}
	}
	printf(KRED"ENDING SENDER"KCYNB"%d",processID);
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
		printf(RED"SIZE: "KCYNB"%d "RED"MAX IN ROOM: "C"%d"RED" LOG: "C"%d\n",
			size, 
			maxInRoom,
			howManySpaces
		);
	int j;
	for(j=0;j<howManySpaces;++j){
		multiplier=multiplier*10;
	}
	setUpRand(processID);
	//testRandom();
	gender = getGender();
	//printf("New process %d %s\n",processID,(gender==0)?"man":"woman");
	//hello world
	pthread_t th;
	pthread_create(&th, NULL,loop,NULL);
	pthread_create(&th, NULL,sender,NULL);

	struct timespec tim2;
	tim2.tv_sec = 1;
	//tim2.tv_nsec = 10000L;
	while(ended != 1) nanosleep(&tim2,NULL);
	MPI_Finalize();
}
