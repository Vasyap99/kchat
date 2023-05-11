
#include "kkoSocket.h"
#include "kko_textsocket.h"
#include <map>
#include <thread>
#include <mutex>
#include <cstdlib>
#include "kko_textfile.h"


#include "kko_ObjectMapper.h"
#include <fstream>
#include <unistd.h>
//#include <winbase.h>

#include "kko_files.h"



#include "mysysinfo.h"


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

class msg{
public:
	string msg1;
	msg(){}
	msg(string m){
		msg1=m;
	}
};
void cacheMsg(const string&LOGIN,const string& msg1){//кэширование сообщения
	remove(("msgs-"+LOGIN+".dat1").c_str());	
		CopyFile(//сохраняем временную копию файла для редактирования
		  	(string("msgs-")+LOGIN+".dat").c_str(),
			(string("msgs-")+LOGIN+".dat1").c_str()
		);				
	//
	fstream f( ("msgs-"+LOGIN+".dat1").c_str() , ios::app | ios::ate );
	ObjectMapper om;
	msg m(msg1);
	om.writeValue(f, &m,"s",{"msg"});
	f.close();

		CopyFile(//заменяем временной копией исходный файл
		  	(string("msgs-")+LOGIN+".dat1").c_str(),
			(string("msgs-")+LOGIN+".dat").c_str()
		);	
}

void loadMsgs(string login,clientstruct *cs){
	ObjectMapper om;
	
	cout << ("msgs-"+login+".dat") <<endl;
	
	fstream f( ("msgs-"+login+".dat").c_str() , ios::in);

	while(!f.eof()){
		cout << ">>>reading value:"<<endl;			
		msg m = om.readValue<msg>(f,"s");
		cout << ">>>OK"<<endl;
		cs->msgs.push_back(m.msg1);
	}	
}


void run(Socket*s){	//нить чтения сообщений
    while(true){
    	try{
			string LOGIN=readS(*s);
			string msg=readS(*s);
			m.lock();
			cacheMsg(LOGIN,string(" >>> ")+msg);			
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
		writeS(s,"A");			cout << "2.1" <<endl;
		writeS(s,login1);			cout << "2.2" <<endl;
		writeS(s,pass1);				cout << "2.3" <<endl;
		res=readS(s);			cout << "2.4" <<endl;
	}catch(...){			cout << "2.5" <<endl;
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
			clientstruct *cs;
			data1.insert(make_pair(login,cs=new clientstruct(FIO)));
			if (fExists("msgs-"+login+".dat") ) 
				loadMsgs(login,cs);
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
		cout << "[Enter command(S-send msg, A-send All, W-show msgs, q-quit)]" << endl;
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
			// Ignore to the end of Stream
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			std::getline(std::cin, MSG);	
			try{
				writeS(s,"S");
				writeS(s,LOGIN);
				writeS(s,MSG);
			}catch(...){
				break;
			}
			m.lock();
			cacheMsg(LOGIN,string(" <<< ")+MSG);
			if(data1.find(LOGIN)!=data1.end() && data1[LOGIN]!=nullptr) data1[LOGIN]->msgs.push_back(string(" <<< ")+MSG);
			m.unlock();	
		}else if(cmd=="A"){
			string MSG;		
			cout << "Enter msg:";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::getline(std::cin, MSG);	
			try{
				writeS(s,"A");
				writeS(s,MSG);
			}catch(...){
				break;
			}
			m.lock();
			for(auto it:data1){
				const string &LOGIN=it.first;
				cacheMsg(LOGIN,string(" <<< ")+MSG);
				if(data1[LOGIN]!=nullptr) data1[LOGIN]->msgs.push_back(string(" <<< ")+MSG);
			}
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

    print_sysinfo();
	
	cc.run1(argc,argv);
	cc.done();
		
	//дожидаемся завершения нити
	//readthread.join();		
		
	//system("pause");
	return 0;
}

