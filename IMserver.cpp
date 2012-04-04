// IMserver.cpp

//Eric Makino

#include <cstring>      // C-style strings, memset
#include <sys/types.h>  // size_t, ssize_t
#include <sys/socket.h> // sockaddr, socket functions
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // htons, inet_pton
#include <unistd.h>     // close
#include <iostream>
#include <string>
#include <errno.h>
#include <netdb.h>
#include "pthread.h"

using namespace std;
#define MAXPENDING 50

struct ThreadArgs{
  int clientSock; //socket to communicate with client
  int serverSock; //server socket
};
struct UserInfo{
	int sock;
	string name;
	bool isOnline;
};

// Global variables
UserInfo users[MAXPENDING];
int counter = 0;
string archive = "";
const bool DEBUG = true;
const string DEBUGmsg = "\t DEBUG: ";

// Function prototypes
void *threadToProcessClient(void *threadArgs);
void processClient(int clientSock);
void clientChoice(int clientSock, int serverSock);
void talk(int clientSock);
void check(int clientSock);
string getNameOfSender(string protocol);
string getNameOfReceiver(string protocol);
string getMsg(string protocol);
void initializeList();
int setUpConnection(unsigned short port);
void sending(int sock,string msg);
void recving(int sock,string &msg);
void error(const char *msg);

int main(int argc, char * argv[])
{
  if(DEBUG) cout << DEBUGmsg << "Debug mode ON." << endl;
  // Start the client will require two command line arguments:
  if(argc < 2){    
    error("USE: ./IMserver <port>\n"); 
  }
  unsigned short servPort = atoi(argv[1]);
  
  initializeList();
  int serverSock = setUpConnection(servPort);
  
  // Accepting multiple connections
  while(true){
	  struct sockaddr_in clientAddr;
	  socklen_t addrLen = sizeof(clientAddr);
	  if(DEBUG) cout << DEBUGmsg << "server currently accepting... " << endl;
	  int clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &addrLen);
	  if(clientSock < 0)
		  error("ERROR accepting client");  
    if(DEBUG) cout << DEBUGmsg << "Client Accepted!" << endl;
    
    // When a client connects, create a new thread to process that client.
	  ThreadArgs* targs = new ThreadArgs;
	  targs -> clientSock = clientSock;
	  targs -> serverSock = serverSock;
	  pthread_t tid;
	  int pthread_Status = pthread_create(&tid, NULL, threadToProcessClient, (void *) targs);
	  if (pthread_Status != 0)
	    error("pthread_create failed");  
    if(DEBUG) cout << DEBUGmsg << "Thread created! thread id: " << tid << endl;
  }
  pthread_exit(NULL);
  return 0;
}

// Thread Function
void *threadToProcessClient(void *threadArgs)
{
  ThreadArgs *threadArgsPtr = static_cast<ThreadArgs *>(threadArgs);
  
  // Insures thread resources are deallocated on return
  pthread_detach(pthread_self());
  // Extract socket file descriptor from argument
  int clientSock = ((ThreadArgs *) threadArgs)->clientSock;
  int serverSock = ((ThreadArgs *) threadArgs)->serverSock;
  delete threadArgsPtr;
  // Communicate with client
  processClient(clientSock);
  clientChoice(clientSock, serverSock);
  close(clientSock);
  //return NULL;
  pthread_exit(NULL);
}

void processClient(int clientSock)
{
  // 1. Receive username from client
  string userName;
  string message;
  bool isNew;

  do{
    isNew = true;
    recving(clientSock, userName);
    // 2. check the name from list
	  for(int i = 0 ; i < MAXPENDING; i++){
		  if(users[i].name == userName && users[i].isOnline == true){
        isNew = false;
        break;
      }
    }
    if(isNew){
      message = userName + " enters the chatroom...";
      sending(clientSock, message);
    }
    else{
      message = "This id is already in use.";
      sending(clientSock, message);
    }
  }while(!isNew);
  
  if(isNew){
	 for(int i = 0 ; i < MAXPENDING; i++){
     if(users[i].isOnline == false){
       users[i].name = userName;
       users[i].sock = clientSock;
       users[i].isOnline = true;
       break;
     }
   }
   message = userName + " joins the server.";
   cout << message << endl;
   for(int i = 0; i<MAXPENDING; i++){
    if(users[i].isOnline == true && users[i].name != userName)
     sending(users[i].sock,message);
   }
	}  
}
void clientChoice(int clientSock, int serverSock)
{
  int n, maxfd;
  fd_set readfds;
  int fd1, fd2;
  fd1 = clientSock;
  fd2 = serverSock;
  maxfd = ( fd1 > fd2 ) ? fd1 : fd2;
  string choice;
  string msg;
  
  while(true){	  
    FD_ZERO(&readfds);
	FD_SET(fd1, &readfds);
	FD_SET(fd2, &readfds);
	n = select(maxfd+1, &readfds, NULL,NULL,NULL);
	if (n < 0){
	  error("could not select");
	}
	else{
	  if (FD_ISSET(fd1, &readfds)){  //client wants to send something
		recving(clientSock, choice);
		if(choice == "1")
		  talk(clientSock);
		else if(choice == "2")
		  check(clientSock);
		else if(choice == "4"){
		  // remove username from list of users
		  close(clientSock);
          for(int i = 0; i < MAXPENDING; i++){
            if(users[i].sock == clientSock){
              msg = users[i].name + " leaves the server.";
              cout << endl << msg << endl;
              users[i].isOnline = false;
              users[i].name = "";
              for(int i = 0; i<MAXPENDING; i++){
                if(users[i].isOnline == true)
                  sending(users[i].sock,msg);
              }
            }
          }
          break;
		}
	  }
	  else if (FD_ISSET(fd2, &readfds))
		{
		  // server wants to write something
		}
	  else
		{
		  // nothing here
		}
	}
  }
}
  
void talk(int clientSock)
{
  for(int i = 0; i < MAXPENDING;i++){
    if(users[i].sock == clientSock)
      users[i].isOnline = false;
  }
  
  int clientSock2;
  bool isValid;
  string msg = "YES";
  string msg2 = "NO";
  string message, sender, receiver, protocol,formatMsg;
  do{
    isValid = false;
    recving(clientSock,receiver);
    for(int i = 0; i < MAXPENDING; i++){
      if(users[i].name == receiver && users[i].isOnline == true){
        isValid = true;
        break;
      }
    }
    if(isValid)
      sending(clientSock,msg);
    else{
      sending(clientSock,msg2);
      check(clientSock);
    }
  }while(!isValid);
  
  recving(clientSock, protocol);
  for(int i = 0; i < MAXPENDING;i++){
    if(users[i].sock == clientSock)
      users[i].isOnline = true;
  }
  cout<<endl<<protocol<<endl;
  
  receiver = getNameOfReceiver(protocol);
  sender = getNameOfSender(protocol);
  message = getMsg(protocol);
  for(int i = 0; i < MAXPENDING;i++){
	// get the socket number for the receiver
	if(users[i].name == receiver)
	  clientSock2 = users[i].sock;
  }
  formatMsg = sender +"->"+receiver+": "+message;
  sending(clientSock2,formatMsg);
}

// check for list of users online
void check(int clientSock)
{
  string ListOfUsers = "Available Users: ";
  for(int i = 0; i< MAXPENDING; i++){
	  if(users[i].isOnline == true && users[i].sock != clientSock){
		  ListOfUsers = ListOfUsers + users[i].name + " ";
		}
	}
  sending(clientSock, ListOfUsers);
}

string getNameOfSender(string protocol)
{
  size_t start, end;
  string name = protocol;
  start = name.find("FROM: ");
  name = name.substr(start+6);
  end = name.find("TO: ");
  name = name.substr(0,end-1);
  return name; 
}

string getNameOfReceiver(string protocol)
{
  size_t start, end;
  string name = protocol;
  start = name.find("TO: ");
  name = name.substr(start+4);
  end = name.find("MSG: ");
  name = name.substr(0,end-1);
  return name; 
}

string getMsg(string protocol)
{
  size_t start, end;
  string msg = protocol;
  start = msg.find("MSG: ");
  msg = msg.substr(start+5);
  end = msg.find("LENGTH: ");
  msg = msg.substr(0,end-1);
  return msg; 
}

void initializeList()
{
  //When the server starts, the listOfUsers starts empty
  for(int i=0; i<MAXPENDING;i++){
	users[i].isOnline = false; //false for offline
  }
}

int setUpConnection(unsigned short port)
{
  unsigned short servPort = port;
  int socketNum;
  
  // 1. Create a TCP socket
  socketNum = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(socketNum == -1)	
	error("ERROR creating the socket");	
  if(DEBUG) cout << DEBUGmsg << "socket created. "
				 << "socketNum: " << socketNum << endl;
  
  // 2. Establish connection
  // Clear out address struct
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  
  // Set the fields
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);
  
  //kill if process still running
  int tr = 1;
  if(setsockopt(socketNum,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1){
    perror("setsockop");
    exit(1);
  }
  // Bind
  int status = bind(socketNum, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status == -1)
	error("ERROR binding");

  // 3. Listen
  status = listen(socketNum, MAXPENDING);
  if(status == -1)
	error("ERROR listening");
  
  return socketNum;
}

void sending(int sock,string msg)
{
  int strLength = msg.length();
  char *message = NULL;
  message = new char[strLength+1];
  strcpy(message, msg.c_str());
  // Send length of name_str
  int bytesSent = send(sock, &strLength, sizeof(int), 0);
  if (bytesSent != sizeof(int)) exit(-1);
  // Send message
  bytesSent = send(sock, (void *) message, strLength, 0);
  if (bytesSent != strLength) exit(-1);
  delete [] message;
}

void recving(int sock,string &msg)
{
  // Receive 
  string message;
  int strLength;
  char c;
  int bytesRecv = recv(sock, &strLength, sizeof(int), 0);
  for (int i = 0; i < strLength; i++) {
	  bytesRecv = recv(sock, (void *)&c, 1, 0);
	  if (bytesRecv <= 0) exit(-1);
	  message = message + c;
  }
  msg = message;
}

void error(const char *msg)
{
  //printf("%s\n", strerror(errno));  // delete this!
  perror(msg);
  exit(-1);
}
