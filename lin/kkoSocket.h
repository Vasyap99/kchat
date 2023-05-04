#include <iostream>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

#include <arpa/inet.h>

void ZeroMemory(void*p, int size){
	for(int i=0;i<size;i++){
		*((char*)p)=0;
		p++;
	}
}

extern "C"{
	//#include <WinSock2.h>
	//#include <WS2tcpip.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>

}
#include <vector>
#include <string>

#include "kpy.h"

namespace kko{

	using namespace std;
	
	/*int inet_pton1(int tp,const char*a,in_addr*ia){
		vector <string> *l=kpy::split(string(a),'.');		
		cout <<"IP::"<<stoi( (*l)[0] )<<endl;
		cout <<"IP::"<<stoi( (*l)[1] )<<endl;		
		cout <<"IP::"<<stoi( (*l)[2] )<<endl;		
		cout <<"IP::"<<stoi( (*l)[3] )<<endl;		
		ia->S_un.S_un_b.s_b1=stoi((*l)[0]);
		ia->S_un.S_un_b.s_b2=stoi((*l)[1]);
		ia->S_un.S_un_b.s_b3=stoi((*l)[2]);
		ia->S_un.S_un_b.s_b4=stoi((*l)[3]);						
		delete l;
		return 1;
	}	
	void inet_ntop1(int tp, in_addr*ia, const char*a, int l){		
	}*/
	
	class SocketError{
		int error;
	public:
		SocketError(int error=0)
			:error(error)
		{}
		int getError(){
			return error;
		}
	};
	
	class ServerSocketError : public SocketError{
	public:
		ServerSocketError(int error=0)
			:SocketError(error)
		{}
	};
	
	//#define inet_pton InetPtonA
	//#define inet_ntop InetNtop

    int Connect(const char *SERVER_IP,unsigned int SERVER_PORT_NUM){
		//Key constants
		///const char SERVER_IP[] = "";					// Enter IPv4 address of Server
		///const short SERVER_PORT_NUM = 0;				// Enter Listening port on Server side
		///const short BUFF_SIZE = 1024;					// Maximum size of buffer for exchange info between server and client

		// Key variables for all program
		int erStat;										// For checking errors in sockets functions

		//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
		in_addr ip_to_num;		
		inet_pton(AF_INET, SERVER_IP, &ip_to_num);


		// WinSock initialization
		//WSADATA wsData;
		//erStat = WSAStartup(MAKEWORD(2,2), &wsData);

		/*if (erStat != 0) {
			cout << "Error Sock version initializaion #";
			//cout << WSAGetLastError();
			///return 1;
			throw SocketError();			
		}else 
			cout << "WinSock initialization is OK" << endl;*/
	
		// Socket initialization
		int ClientSock = socket(AF_INET, SOCK_STREAM, 0);

		if (ClientSock < 0) {
			cout << "Error initialization socket # " << endl;
			::close(ClientSock);
			//WSACleanup();
			throw SocketError();			
		}else 
			cout << "Client socket initialization is OK" << endl;

		// Establishing a connection to Server
		sockaddr_in servInfo;

		ZeroMemory(&servInfo, sizeof(servInfo));

		servInfo.sin_family = AF_INET;
		servInfo.sin_addr = ip_to_num;	
		servInfo.sin_port = htons(SERVER_PORT_NUM);

		erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
	
		if (erStat <0) {
			cout << "Connection to Server is FAILED. Error # " << endl;
			::close(ClientSock);
			//WSACleanup();
			///return 1;
			throw SocketError();
		}else{
			cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;
			return ClientSock;
		}
	}

#include <sys/ioctl.h>

bool isclosed(int sock) {
  fd_set rfd;
  FD_ZERO(&rfd);
  FD_SET(sock, &rfd);
  timeval tv = { 0 };
  select(sock+1, &rfd, 0, 0, &tv);
  if (!FD_ISSET(sock, &rfd))
    return false;
  int n = 0;
  ioctl(sock, FIONREAD, &n);
  return n == 0;
}


	class Socket{
		const short BUFF_SIZE;
		int ClientConn;
	public:
		Socket(int S)
			:BUFF_SIZE(1024)
		{
			ClientConn=S;
		}
		string recv(int n){
			string res="";			
			while(res.length()<n){
				vector <char> buff(n-res.length()); 
				int packet_size = ::recv(ClientConn, buff.data(), buff.size(), 0);
				if(packet_size<0 || isclosed(ClientConn)){
					throw SocketError();
				}else{
					res+=string(buff.data()).substr(0,packet_size);
				}
			}
			return res;
		}
		int send(string s){
			//vector <char> buff(s.size());	
			while(s.length()>0){
				int packet_size = ::send(ClientConn, /*buff.data()*/s.c_str(), /*buff.size()*/s.length(), 0);
				if(packet_size<0 || isclosed(ClientConn)){
					throw SocketError();
				}else{
					s=s.substr(packet_size,s.length()-packet_size);
				}
			}
		}
		void close(){
			::close(ClientConn);
		}
	};
	
	
	class ServerSocket{
		int ServSock;
		string IP_SERV;			// Enter local Server IP address
		const int PORT_NUM;				// Enter Open working server port
		// Key variables for all program
		int erStat;								// Keeps socket errors status

		//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
		///in_addr ip_to_num;
        struct sockaddr_in addr;
		
		// WinSock initialization
		//WSADATA wsData;		
	public:
		ServerSocket(const char *ip="127.0.0.1",unsigned int port=8087)
			:IP_SERV(ip),PORT_NUM(port)
		{
			//in_ddr ip_to_num;
			//erStat = inet_pton(AF_INET, IP_SERV.c_str(), &ip_to_num);
	
			/*if (erStat <= 0) {
				cout << "Error in IP translation to special numeric format" << endl;
				///return 1;
			}
			
			erStat = WSAStartup(MAKEWORD(2,2), &wsData);
	
			if ( erStat != 0 ) {
				cout << "Error WinSock version initializaion #";
				cout << WSAGetLastError();
				///return 1;
			}else 
			    cout << "WinSock initialization is OK" << endl;*/
			
			// Server socket initialization

		    addr.sin_family = AF_INET;
    		addr.sin_port = htons(port);
		    addr.sin_addr.s_addr = htonl(INADDR_ANY);
		
			ServSock = socket(AF_INET, SOCK_STREAM, 0);

			if (ServSock <0) {
				cout << "Error initialization socket # " << endl; 
				::close(ServSock);
				//WSACleanup();
				///return 1;
			}else
				cout << "Server socket initialization is OK" << endl;			
						
		}
		void bind(){
			// Server socket binding
			/*sockaddr_in servInfo;
			ZeroMemory(&servInfo, sizeof(servInfo));	// Initializing servInfo structure
				
			servInfo.sin_family = AF_INET;
			servInfo.sin_addr = ip_to_num;	
			servInfo.sin_port = htons(PORT_NUM);*/

			erStat = ::bind(ServSock, (sockaddr*)&addr, sizeof(addr));

			if ( erStat != 0 ) {
				cout << "Error Socket binding to server info. Error # "  << endl;
				::close(ServSock);
				//WSACleanup();
				throw ServerSocketError();
				///return 1;
			}else 
				cout << "Binding socket to Server info is OK" << endl;

		}
		
		void listen(){
			//Starting to listen to any Clients
			erStat = ::listen(ServSock, SOMAXCONN);

			if ( erStat != 0 ) {
				cout << "Can't start to listen to. Error # "  << endl;
				::close(ServSock);
				//WSACleanup();
				throw ServerSocketError();
				///return 1;
			}else{
				cout << "Listening..." << endl;
			}			
		}
		
		Socket accept(){
			//Client socket creation and acception in case of connection
			sockaddr_in clientInfo; 
			ZeroMemory(&clientInfo, sizeof(clientInfo));	// Initializing clientInfo structure

			socklen_t clientInfo_size = sizeof(clientInfo);

			int ClientConn = ::accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

			if (ClientConn <0 ) {
				cout << "Client detected, but can't connect to a client. Error # " << endl;
				::close(ServSock);
				::close(ClientConn);
				//WSACleanup();
				throw ServerSocketError();
				///return 1;
			}else{
				cout << "Connection to a client established successfully" << endl;
				//char clientIP[22];

				//inet_ntop1(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);	// Convert connected client's IP to standard string format

				//cout << "Client connected with IP address " << clientIP << endl;
				
				return Socket(ClientConn);
			}			
		}
		void close(){
			::close(ServSock);
		}
	};	
	

}

