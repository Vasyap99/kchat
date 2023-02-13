namespace kko{

string readS(Socket &s){
	string r="";
	while(true){
		string b=s.recv(1);
		if(b!="\n" && b!="\r") r+=b;
		if(b=="\n") break;
	}
	return r;
}
void writeS(Socket &s,const string &d){
	s.send(d+"\n");
}
void writeS(Socket &s,const string &&d){
	s.send(d+"\n");
}

}
