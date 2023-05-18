#include <iostream>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

//#include <arpa/inet.h>

extern "C"{
	#include <WinSock2.h>
	#include <WS2tcpip.h>
}
#include <vector>
#include <string>

#include "kpy.h"

#include "kkoTime.h"

namespace kko{

	using namespace std;
	
	int inet_pton1(int tp,const char*a,in_addr*ia){
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
	}
	
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

    SOCKET Connect(const char *SERVER_IP,unsigned int SERVER_PORT_NUM){
		//Key constants
		///const char SERVER_IP[] = "";					// Enter IPv4 address of Server
		///const short SERVER_PORT_NUM = 0;				// Enter Listening port on Server side
		///const short BUFF_SIZE = 1024;					// Maximum size of buffer for exchange info between server and client

		// Key variables for all program
		int erStat;										// For checking errors in sockets functions

		//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
		in_addr ip_to_num;		
		inet_pton1(AF_INET, SERVER_IP, &ip_to_num);


		// WinSock initialization
		WSADATA wsData;
		erStat = WSAStartup(MAKEWORD(2,2), &wsData);

		if (erStat != 0) {
			cout << "Error WinSock version initializaion #";
			cout << WSAGetLastError();
			///return 1;
			throw SocketError();			
		}else 
			cout << "WinSock initialization is OK" << endl;
	
		// Socket initialization
		SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

		if (ClientSock == INVALID_SOCKET) {
			cout << "Error initialization socket # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
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
	
		if (erStat != 0) {
			cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			///return 1;
			throw SocketError();
		}else{
			cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;
			return ClientSock;
		}
	}

	class Socket{
		const short BUFF_SIZE;
		SOCKET ClientConn;
	public:
		Socket(SOCKET S)
			:BUFF_SIZE(1024)
		{
			ClientConn=S;
		}
		string recv(int n){
			int ms=0;
			string res="";			
			while(res.length()<n){
				vector <char> buff(n-res.length()); 
				int packet_size = ::recv(ClientConn, buff.data(), buff.size(), 0);
                // 
				if(packet_size>0) ms=0;
				else{
					if(ms==0) ms=1;
                    if(ms<1000) ms*=2;
					sleep(ms);
				}
                // 				
				if(packet_size==SOCKET_ERROR){
					throw SocketError();
				}else{
					res+=string(buff.data()).substr(0,packet_size);
				}
			}
			return res;
		}
		int send(string s){
			int ms=0;
			//vector <char> buff(s.size());	
			while(s.length()>0){
				int packet_size = ::send(ClientConn, /*buff.data()*/s.c_str(), /*buff.size()*/s.length(), 0);
                // 
				if(packet_size>0) ms=0;
				else{
					if(ms==0) ms=1;
                    if(ms<1000) ms*=2;
					sleep(ms);
				}
                // 
				if(packet_size==SOCKET_ERROR){
					throw SocketError();
				}else{
					s=s.substr(packet_size,s.length()-packet_size);
				}
			}
		}
		void close(){
			closesocket(ClientConn);
		}
	};
	
	
	class ServerSocket{
		SOCKET ServSock;
		string IP_SERV;			// Enter local Server IP address
		const int PORT_NUM;				// Enter Open working server port
		// Key variables for all program
		int erStat;								// Keeps socket errors status

		//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
		in_addr ip_to_num;
		
		// WinSock initialization
		WSADATA wsData;		
	public:
		ServerSocket(const char *ip="127.0.0.1",unsigned int port=8087)
			:IP_SERV(ip),PORT_NUM(port)
		{
			//in_ddr ip_to_num;
			erStat = inet_pton1(AF_INET, IP_SERV.c_str(), &ip_to_num);
	
			if (erStat <= 0) {
				cout << "Error in IP translation to special numeric format" << endl;
				///return 1;
			}
			
			erStat = WSAStartup(MAKEWORD(2,2), &wsData);
	
			if ( erStat != 0 ) {
				cout << "Error WinSock version initializaion #";
				cout << WSAGetLastError();
				///return 1;
			}else 
			    cout << "WinSock initialization is OK" << endl;
			
			// Server socket initialization
			ServSock = socket(AF_INET, SOCK_STREAM, 0);

			if (ServSock == INVALID_SOCKET) {
				cout << "Error initialization socket # " << WSAGetLastError() << endl; 
				closesocket(ServSock);
				WSACleanup();
				///return 1;
			}else
				cout << "Server socket initialization is OK" << endl;			
						
		}
		void bind(){
			// Server socket binding
			sockaddr_in servInfo;
			ZeroMemory(&servInfo, sizeof(servInfo));	// Initializing servInfo structure
				
			servInfo.sin_family = AF_INET;
			servInfo.sin_addr = ip_to_num;	
			servInfo.sin_port = htons(PORT_NUM);

			erStat = ::bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));

			if ( erStat != 0 ) {
				cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
				closesocket(ServSock);
				WSACleanup();
				throw ServerSocketError();
				///return 1;
			}else 
				cout << "Binding socket to Server info is OK" << endl;

		}
		
		void listen(){
			//Starting to listen to any Clients
			erStat = ::listen(ServSock, SOMAXCONN);

			if ( erStat != 0 ) {
				cout << "Can't start to listen to. Error # " << WSAGetLastError() << endl;
				closesocket(ServSock);
				WSACleanup();
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

			int clientInfo_size = sizeof(clientInfo);

			SOCKET ClientConn = ::accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

			if (ClientConn == INVALID_SOCKET) {
				cout << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
				closesocket(ServSock);
				closesocket(ClientConn);
				WSACleanup();
				throw ServerSocketError();
				///return 1;
			}else{
				cout << "Connection to a client established successfully" << endl;
				char clientIP[22];

				inet_ntop1(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);	// Convert connected client's IP to standard string format

				cout << "Client connected with IP address " << clientIP << endl;
				
				return Socket(ClientConn);
			}			
		}
		void close(){
			closesocket(ServSock);
		}
	};	
	

}

