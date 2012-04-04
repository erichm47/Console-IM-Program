// IMclient.cpp

// Eric Makino

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
#include <sstream>
using namespace std;

// global variables
const int STDIN = 0; /* stdin file descriptor */
const bool DEBUG = true; //false;
const string DEBUGmsg = "\t DEBUG: ";


// Function Prototypes
void myIMapp(int socketNum,string &name);
void talk(int socketNum,string &name);
void check(int socketNum);
int connectionSetUp(char* host,unsigned short port);
void sending(int sock,string msg);
void recving(int sock,string &msg);
void error(const char *msg);

int main(int argc, char *argv[])
{
  if(DEBUG) cout << DEBUGmsg << "Debug mode ON." << endl;
  // Start the client will require two command line arguments:
  if(argc < 3){    
    error("USE: ./IMclient <hostname> <port>\n"); 
  }
  char *hostname = argv[1];
  unsigned short servPort = atoi(argv[2]);

  /* declaring select structures */
  fd_set readFD;
  fd_set master;
  FD_ZERO(&readFD);
  FD_ZERO(&master);
  FD_SET(STDIN, &master); //adding stdin to master

  int socketNum = connectionSetUp(hostname,servPort);
  int fdmax = socketNum;
  FD_SET(socketNum, &master); // add socket to master
  string name;
  // connect with server
  myIMapp(socketNum,name); 
  FD_CLR(socketNum, &master);
  fdmax = STDIN;
  string option("");
  int count = 1;
  string menu = "1)Talk\n2)Check user online\n3)Menu\n4)Quit\n";
  cout<<menu<<endl;
  cout << "Enter choice (1,2,3,4): " << endl;
  while(true)
	{
	  FD_SET(socketNum,&master); /* add sockfd to master */     
	  FD_SET(socketNum,&master); /* add STDIN to master */     
	  fdmax=socketNum; 
	  readFD = master;
	  if(select(fdmax+1, &readFD, NULL, NULL, NULL) == -1){
		cout << "Error occured, closing program..." <<  endl;
		return 1;
	  }
	  else{
		if(FD_ISSET(STDIN, &readFD)){ // standard input
		  if (count == 1){
			cin.ignore();
			count++;
		  }
		  getline(cin, option);
		  if(option == "1"){ // talk
			sending(socketNum,option);
			talk(socketNum,name);
		  }
		  else if(option == "2"){ // check user online
			sending(socketNum,option);
			check(socketNum);
		  }
		  else if(option == "3"){
			cout<<menu<<endl;
		  }
		  else if(option == "4") // quit
			{
			  sending(socketNum,option);
			  cout << "Thank you for using myIM" << endl;
			  break;
			}
		  else
			{
			}
		  
		  cout << "Enter choice (3 for MENU): " << endl;
		}
		else if(FD_ISSET(socketNum, &readFD)){ // receive something from server
		  string message = "";
		  recving(socketNum, message);
		  cout << message << endl;
		}
		else
		  {
			// nothing happens here
		  }
	  }
	}
  // Close
  close(socketNum);
  exit(0);
}

void myIMapp(int socketNum,string &name)
{
  bool duplicate;
  string userName;
  //Welcome to myIM!
  cout << endl;
  cout << "**************************" << endl;
  cout << "*                        *" << endl;
  cout << "*   Welcome to myIM!     *" << endl;
  cout << "*                        *" << endl;
  cout << "**************************\n\n" << endl;
  
  do
	{
	  duplicate = false;
	  string message;
	  
	  cout << "Enter your username: ";
	  cin  >> userName;
	  sending(socketNum, userName);
	  recving(socketNum, message);
	  cout<<message<<endl;
	  if(message == "This id is already in use.")
		duplicate = true;
	}while(duplicate);
  name = userName;
}

void talk(int socketNum, string &username)
{
  string name;
  string msg;
  string Protocol;
  int leng;
  string len;
  bool isValid;
  
  do{
	isValid = true;
	cout<<"Talk to: ";
	cin>>name;
    
	sending(socketNum,name);//check valid name
	recving(socketNum,msg);
	if(msg == "NO")
      {
        isValid = false;
        cout<<"Please enter valid username."<<endl;
        check(socketNum);
      }
  }while(!isValid);
  
  cout<<"Message: ";
  cin.ignore();
  getline(cin,msg);
  leng = msg.length();
  stringstream temp;
  temp << leng;
  len = temp.str();
  Protocol = "FROM: "+username+"\nTO: "+name+"\nMSG: "+msg+"\nLENGTH: "+len+"\n";
  sending(socketNum,Protocol);
}
void check(int socketNum)
{
  string list;
  recving(socketNum,list);
  cout<<"************************" << endl;
  cout<<list<<endl;
  cout<<"************************" << endl;
}
int connectionSetUp(char* host,unsigned short port)
{
  int i = 0;
  char *hostname = host;
  unsigned short servPort = port;
  if(DEBUG) cout << DEBUGmsg << "hostname: " << hostname << " - port: " << servPort << endl;
  
  // Get address by hostname
  const char *pHost = hostname;
  struct hostent *hen = gethostbyname(pHost);
  if(!hen)
    error("ERROR getting the hostname");
  
  struct in_addr **addr_list;
  addr_list = (struct in_addr **)gethostbyname(hostname)->h_addr_list;
  char *IPAddr;
  for(i = 0; addr_list[i] != NULL; i++){ 
	IPAddr = inet_ntoa(*addr_list[i]);
  }
  
  // 1. Create a TCP socket
  int socketNum = socket(AF_INET, SOCK_STREAM, 0);
  if(socketNum == -1)	
	error("ERROR creating the socket");	
  if(DEBUG) cout << DEBUGmsg << "socket created. " << "socketNum: " << socketNum << endl;
  
  // 2. Establish connection 
  // Convert dotted decimal address to int
  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, &servIP);
  if(status <= 0)
    error("ERROR converting IP");
  
  // Clear out address struct
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  
  // Set the fields
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(servPort);
  
  if(DEBUG) cout << DEBUGmsg << "Trying " << IPAddr << endl;
  
  // 3. Connect
  status = connect(socketNum, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status == -1)
    error("ERROR connecting to the server");
  else{
    if(DEBUG) cout << DEBUGmsg << "Connected to " << IPAddr;
  }
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
  // Send name to server
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
  perror(msg);
  exit(-1);
}
