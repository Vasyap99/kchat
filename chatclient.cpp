
#include "kkoSocket.h"
#include "kko_textsocket.h"
#include <map>
#include <thread>
#include <mutex>
#include <cstdlib>
#include "kko_textfile.h"


using namespace kko;
using namespace std;



string readAddr(){
	FILE *f=fopen("addr.dat","rb");
	string  addr=readS(f);
	fclose(f);
	return addr;
}
string readPort(){
	FILE *f=fopen("port.dat","rb");
	string  port=readS(f);
	fclose(f);
	return port;
}

struct clientstruct{
	string fio;
	vector<string> msgs;
	clientstruct(string d)
		:fio(d)
	{}
};


class chatclient{
	Socket s;
	bool d;
public:
	chatclient()
		:s(Connect( readAddr().c_str(),stoi(readPort()) )), d(false)
	{}
	void run1(int argc, char** argv);
	void done();
};
chatclient cc;

map <string,clientstruct*> data1;
mutex m,m2;
volatile bool sgn(false);
void run(Socket*s){	//нить чтения сообщений
    while(true){
    	try{
			string LOGIN=readS(*s);
			string msg=readS(*s);
			m.lock();
			if(data1.find(LOGIN)!=data1.end() && data1[LOGIN]!=nullptr)
				data1[LOGIN]->msgs.push_back(string(" >>> ")+msg);
			m.unlock();		
		}catch(...){
			break;
		}
	}
	cc.done();
}



void chatclient::run1(int argc, char** argv){
	if(argc>1){ //регистрация
		///cout << "1.0 connecting" << endl;		
		///Socket s(Connect("127.0.0.1",8087));

		//чтение файла конфигурации
		cout << "1.1 reading config" << endl;		
		FILE *f=fopen("client.dat","rb");
		string  login1=readS(f),
				pass1=readS(f),
				fio=readS(f);		
		fclose(f);

		//регистрируемся
		cout << "2 registration" << endl;	
		string res;
		try{
			writeS(s,"R");
			writeS(s,login1);
			writeS(s,pass1);	
			writeS(s,fio);
			res=readS(s);			
		}catch(...){
			return;
		}
		if(res!="ok"){
			cout << "reg error" << endl;
			return;
		}else{
			cout << "reg ok" << endl;
		}
	
		//system("pause");
		return;
	}
	
	cout << "1.0 connecting" << endl;		
	//Socket s(Connect("127.0.0.1",8087));
	//s.send("hello2");
	//std::cout << s.recv(8);

	//чтение файла конфигурации
	cout << "1.1 reading config" << endl;		
	FILE *f=fopen("client.dat","rb");
	string  login1=readS(f),
			pass1=readS(f);		
	fclose(f);
	//
		
	//логинимся
	cout << "2 logining" << endl;	
	string res;
	try{
		writeS(s,"A");
		writeS(s,login1);
		writeS(s,pass1);	
		res=readS(s);
	}catch(...){
		return;
	}
	if(res!="ok"){
		cout << "auth error" << endl;
		return;
	}else{
		cout << "auth ok" << endl;
	}
	
	//чтение списка пользователей
	cout << "3 reading users list" << endl;		
	try{
		while(true){
			string d=readS(s);
			if(d=="") break;
			string login=d;
			string FIO=readS(s);
			data1.insert(make_pair(login,new clientstruct(FIO)));
			cout << "reading users list: " << login << FIO << endl;
		}	
	}catch(...){
		return;
	}
	
	//создаем нить чтения сообщений
	cout << "4 creating msg reading thread" << endl;		
	thread readthread(run,&s);
	readthread.detach();
	
	//цикл обработки команд(вывод сообщений и отправка сообщений)
	cout << "5 waiting for cmds" << endl;			
	while(true){
		if(sgn) break;
		string cmd;
		cout << "[Enter command(S-send msg, W-show msgs, q-quit)]" << endl;
		cin >> cmd;
		if(cmd=="q"){
			s.close();
			break;
		}else if(cmd=="S"){
			string LOGIN;
			string MSG;		
			cout << "Enter user ID:";
			cin >> LOGIN;			
			cout << "Enter msg:";
			cin >> MSG;	
			try{
				writeS(s,"S");
				writeS(s,LOGIN);
				writeS(s,MSG);
			}catch(...){
				break;
			}
			m.lock();
			if(data1.find(LOGIN)!=data1.end() && data1[LOGIN]!=nullptr) data1[LOGIN]->msgs.push_back(string(" <<< ")+MSG);
			m.unlock();				
		}else if(cmd=="W"){
			m.lock();
			map <string, clientstruct*> :: iterator it = data1.begin();
		    for (int i = 0; it != data1.end(); it++, i++) if(it->second!=nullptr){ 
        		cout << it->second->fio << ':' << endl;
				for(auto ms: it->second->msgs){
					cout << "-" << ms << endl;
				}
    		}/**/
			m.unlock();    		
		}else{
			cout << "[Unkn command]" << endl;
		}
	}
	
}

	void chatclient::done(){
		m2.lock();
		if(!d){

        d=true;
		cout << "~B" << endl;
		s.close();
		cout << "~1" << endl;		
		cout << "~2" << endl;		
		m.lock();
		cout << "~3" << endl;		
		for(auto i:data1) if(i.second!=nullptr){
			delete i.second;
		}
		m.unlock();
		cout << "~E" << endl;		
	
		}
		m2.unlock();
		//system("pause");		
	}


int main(int argc, char** argv) {
	setlocale (LC_ALL,"Russian");
	
	cc.run1(argc,argv);
	cc.done();
		
	//дожидаемся завершения нити
	//readthread.join();		
		
	//system("pause");
	return 0;
}

