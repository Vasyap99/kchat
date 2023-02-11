
#include "kkoSocket.h"
#include "kkoSemaphore.h"

#include <cstdlib>
#include <unistd.h>
#include <map>
#include <thread>
#include <mutex>

using namespace kko;

struct serverstruct{
	string fio;
	string login;
	string passw;
	Socket * volatile s;
	thread * volatile t;
public:
	void setS(Socket*s){
		this->s=s;
	}
	serverstruct(string fio,string login,string psw)
		:fio(fio),login(login),passw(psw),s(nullptr),t(nullptr)
	{}
};

string readS(FILE *f){
	string s;
	char b;
	while(true){
		fread(&b,1,1,f);
		if(b!='\r' && b!='\n' && !feof(f)) s+=b;
		else if(b=='\n' || feof(f)) break;
	}
	return s;
}

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
void writeS(FILE*f,string s){
	s+="\r\n";
	fputs(s.c_str(),f);
}


class chatserver{
	ServerSocket ss;
public:
	//chatserver(){}
	void run1();
	done();
};
chatserver cs1;


map <string,serverstruct*> data1;
Semaphore m  //блокировка на саму data1 и паралельное использование сокетов;
     ,mf //блокировка на файлы сообщений
	 ,m2;//блокировка на serverstruct-ы	(их удаление,очистку и заполнение)

class tmpException{};

void run(serverstruct *d){//нить обработки соединения с клиентом
	try{
		//отправка списка пользователей
		m.lock();
		for(auto ms: data1){
			try{
				writeS(*(d->s),ms.second->login);
				writeS(*(d->s),ms.second->fio);				
			}catch(...){
				m.unlock();
				throw tmpException();
			}
		}
		m.unlock();
		writeS(*(d->s),"");			//(пустая строка означает конец списка)
		//отправляем закэшированные сообщения
		if (access(("msgs-"+d->login+".dat").c_str(), F_OK) == 0) { 				// file exists?
			mf.lock();
			FILE *f=fopen((string("msgs-")+d->login+".dat").c_str(),"rb");
			while(true){
				if(feof(f)) break;
				string login0,msg0;
				//
				try{
					login0=readS(f);
					if(login0=="") break;
					msg0=readS(f);									
					m.lock();
					writeS(*(d->s),login0);	
					writeS(*(d->s),msg0);					
					m.unlock();					
				}catch(...){
					fclose(f);					
					mf.unlock();
					m.unlock();										
					throw tmpException();
				}
				cout << "SEND Cached:" <<d->login <<' ' <<login0 <<' ' <<msg0 <<endl;				
			}
			fclose(f);
			remove(("msgs-"+d->login+".dat").c_str());
			mf.unlock();		 				
		}
		//-цикл пересылки сообщений
		while(true){			
			readS(*(d->s)); //"S"
			string LOGIN=readS(*(d->s));
			string msg=readS(*(d->s));
			//проверяем, есть ли элемент с таким индексом
			m.lock();
			if(data1.find(LOGIN)==data1.end()){//нет такого индекса
				m.unlock();
				continue;
			}	
			m2.lock();
			if(data1[LOGIN]!=nullptr && data1[LOGIN]->s!=nullptr){		//если клиент подключен, передаем ему
				try{
					writeS(*(data1[LOGIN]->s),d->login);	
					writeS(*(data1[LOGIN]->s),msg);
				}catch(...){
					m.unlock();
					m2.unlock();
					throw tmpException();
				}
				m.unlock();
				m2.unlock();
			}else{  								//иначе пишем в файл
				m.unlock();
				m2.unlock();
				mf.lock();
				FILE *f=fopen((string("msgs-")+LOGIN+".dat").c_str(),"ab");
				writeS(f,d->login);
				writeS(f,msg);
				fclose(f);
				mf.unlock();
			}
		}					
	}catch(...){ //разрываем соединение и удаляем объект нити
		cout << "!run() ex!" <<endl;
		{
			m2.lock();
			try{
				d->s->close();//закрываем сокет
			}catch(...){}
			delete d->s; //удаляем объект сокета
			d->s=nullptr;
			d->t->detach();//отсоединяем нить от объекта нити
			delete d->t; //удаляем объект нити
			d->t=nullptr;
			m2.unlock();			
		}
		cout << "!run() ex-E!" <<endl;
		//break; //выходим из цикла, завершая нить
	}
}


	void chatserver::run1(){
		setlocale (LC_ALL,"Russian");
	
		//чтение файла бд пользователей
		FILE *f=fopen("serverdb.dat","rb");
    	while(true){
    		string fio,login,pass;
    		login=readS(f);
    		if(login=="") break;
    		pass=readS(f);
    		fio=readS(f);
    		data1.insert(make_pair(login,new serverstruct(fio,login,pass)));
    		cout << fio << " : " << login << " : " << pass << endl;
		}
		fclose(f);
		//
	
    	ss.bind();
		while(true){
			ss.listen();
			Socket *s=new Socket(ss.accept());
			string command;
			try{
				command=readS(*s);				
			}catch(...){
				delete s;
				continue;
			}
			if(command=="A"){//команда авторизации клиента
				//авторизация клиента
				string login,pass;
				try{
					login=readS(*s);		
					pass=readS(*s);				
				}catch(...){
					delete s;
					continue;					
				}
				m.lock(); m2.lock();
				bool b=true,b1=true;// b-есть такой пользователь, b1-совпадают логин и пароль и нет подключения
				try{
					if(data1.find(login)==data1.end()) b=false; //(нет элемента с таким индексом?)
					else{
						if(data1[login]!=nullptr && data1[login]->login==login && data1[login]->passw==pass && data1[login]->t==nullptr){
							data1[login]->setS(s);
						}else b1=false;
					}
				}catch(...){//исключение - нет такого пользователя
					b=false;
				}
				if(b && b1){
					try{
						writeS(*s,"ok"); //авторизация успешна
					}catch(...){
						m.unlock();	 m2.unlock();					
						s->close();
						delete s;
						continue;						
					}
				}else{
					try{
						writeS(*s,"err");  //авторизация неуспешна
					}catch(...){}
					m.unlock();	 m2.unlock();				
					s->close();
					delete s;
					continue;
				}
				thread *t=new thread(run,data1[login]);
				data1[login]->t=t;
				m.unlock();  m2.unlock();
				cout << "Auth:" << login << pass << endl;
			}else if(command=="R"){//регистрация
				string login,pass,fio;			
				bool b=true,b1=true;  //(b-нет сетевой ошибки, b1-нет пользователя с таким именем)
				try{
					login=readS(*s);		
					pass=readS(*s);
					fio=readS(*s);	
					try{
						m.lock();
						if(data1.find(login)!=data1.end()) b1=false;
						m.unlock();						
					}catch(...){}
				}catch(...){
					b=false;
				}			
				if(b && b1){//регистрация
					//
					FILE *f=fopen("serverdb.dat","ab");
					writeS(f,login);
					writeS(f,pass);			
					writeS(f,fio);			
					fclose(f);
					//
					try{
						writeS(*s,"ok");
					}catch(...){}	
					delete s;									
					//
					this->done(); //завершаем сервер					
				}else{
					try{
						writeS(*s,"err");
					}catch(...){}	
					delete s;									
					//					
				}

			}else{//неизвестная команда
				delete s;
			}
		}
	}
	chatserver::done(){
		cout << "~chatserver()-B" << endl;
		m.lock();
		for(auto i:data1) if(i.second!=nullptr){
			if(i.second->s!=nullptr) {// закрываем сокет(приводит к завершению нити)
				i.second->s->close();
			}
			while(i.second->t!=nullptr){}	
			m2.lock();
			delete i.second;			
			data1[i.first]=nullptr;
			m2.unlock();			
		}
		m.unlock();					
		cout << "~chatserver()-E" << endl;
		//system("pause");
		ss.close();
	}


void t1(){//нить чтения консольных команд для сервера
	string s;
	while(true){
		cin>>s;
		if(s=="q"){
			cs1.done();
			break;
		}
	}
}

int main(int argc, char** argv) {
	thread tt(t1);
	try{
		cs1.run1();
	}catch(kko::SocketError e){
		cout << "!exception in main thread!" <<endl;
	}catch(...){
		cout << "!UNKNOWN exception in main thread!" <<endl;
	}
	tt.join();
	return 0;
}

