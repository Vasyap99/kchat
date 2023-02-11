
#include "kkoSocket.h"

using namespace kko;

string readS(Socket s){
	string r="";
	while(true){
		string b=s.recv(1);
		if(b!="\n" && b!="\r") r+=b;
		if(b=="\n") break;
	}
	return r;
}
void writeS(Socket s,string d){
	s.send(d+"\n");
}

void mysend(Socket os1/*,CT,fl*/){
	/*
    #print "sending file:",fl
    #print "mimetype:",CT
    #CT="text/html"#"application/octet-stream"
    try:
	data=open("."+fl).read()#"<html><body>kok"
    except:
	data=""
	*/
	string data="test";
	string CT="text/html";
	
    int siz=data.length();
    writeS(os1,"HTTP/1.1 200 OK");
    writeS(os1,"Server: nginx/1.0.11");
    writeS(os1,"Date: Thu, 19 Feb 2009 11:08:01 GMT");
    writeS(os1,"Content-Type: "+CT);
    writeS(os1,"Content-Length: "+std::to_string(siz));
    writeS(os1,"Last-Modified: Sat, 21 Feb 2009 11:08:01 GMT");
    writeS(os1,"Connection: close");
    writeS(os1,"Content-Disposition: inline");
    writeS(os1,"Pragma: public");
    writeS(os1,"Accept-Ranges: bytes");
    //#//writeS(os1,"Content-Enc oding: identity");
    writeS(os1,"");
    writeS(os1,data);
}

int main(int argc, char** argv) {
	ServerSocket ss;
    ss.bind();
	while(true){
		ss.listen();
		Socket s=ss.accept();
		while(true){
			string d=readS(s);
			cout << "(got str):" << d << "\r\n";
			if(d=="") break;
		}
		
		mysend(s);
		
		//std::cout << s.recv(6);
		//s.send("<string>");
	}
	return 0;
}

