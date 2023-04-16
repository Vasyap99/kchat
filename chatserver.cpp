
#include "kkoSocket.h"
#include "kko_textsocket.h"
#include "kkoSemaphore.h"

#include <cstdlib>
#include "kko_textfile.h"
#include <unistd.h>
#include <winbase.h>
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


class chatserver{
	ServerSocket ss;
public:
	//chatserver(){}
	void run1();
	done();
};
chatserver cs1;


map <string,serverstruct*> data1;
Semaphore m  //���������� �� ���� data1 � ����������� ������������� �������;
     ,mf //���������� �� ����� ���������
	 ,m2;//���������� �� serverstruct-�	(�� ��������,������� � ����������)

class tmpException{};

void cacheMsg(serverstruct *d,const string& msg,const string& LOGIN){//����������� ���������
	mf.lock();
	remove(("msgs-"+LOGIN+".dat1").c_str());	
		CopyFile(//��������� ��������� ����� ����� ��� ��������������
		  	(string("msgs-")+LOGIN+".dat").c_str(),
			(string("msgs-")+LOGIN+".dat1").c_str(),
			false
		);				
	FILE *f=fopen((string("msgs-")+LOGIN+".dat1").c_str(),"ab");
	writeS(f,d->login);
	writeS(f,msg);
	fclose(f);
		CopyFile(//�������� ��������� ������ �������� ����
		  	(string("msgs-")+LOGIN+".dat1").c_str(),
			(string("msgs-")+LOGIN+".dat").c_str(),
			false
		);				
	mf.unlock();
	;
}

void sendMsg(serverstruct *d,const string& msg,const string& LOGIN){//�������� ��������� ������������
			//���������, ���� �� ������� � ����� ��������
			m.lock();
			if(data1.find(LOGIN)==data1.end()){//��� ������ �������
				m.unlock();
				return; //continue
			}	
			m2.lock();
			if(data1[LOGIN]!=nullptr && data1[LOGIN]->s!=nullptr){		//���� ������ ���������, �������� ���
				try{
					writeS(*(data1[LOGIN]->s),d->login);	
					writeS(*(data1[LOGIN]->s),msg);
				}catch(...){
					cacheMsg(d,msg,LOGIN);
					//m.unlock();
					//m2.unlock();
					//throw tmpException();
				}
				m2.unlock();				
				m.unlock();
			}else{  								//����� ����� � ����
				m2.unlock();			
				m.unlock();
				cacheMsg(d,msg,LOGIN);
			}
}

void sendMsgAll(serverstruct *d,const string& msg){//�������� ��������� ���� ������������� (�����������������)
			//���������, ���� �� ������� � ����� ��������
			m.lock();
			m2.lock();			
			for(auto &it:data1){
			
			string &LOGIN=it.second->login;

			
			/*if(data1.find(LOGIN)==data1.end()){//��� ������ �������
				continue;
			}*/	
			if(data1[LOGIN]!=nullptr && data1[LOGIN]->s!=nullptr){		//���� ������ ���������, �������� ���
				try{
					writeS(*(data1[LOGIN]->s),d->login);	
					writeS(*(data1[LOGIN]->s),msg);
				}catch(...){
					cacheMsg(d,msg,LOGIN);
					//m.unlock();
					//m2.unlock();
					//throw tmpException();
				}
			}else{  								//����� ����� � ����
				cacheMsg(d,msg,LOGIN);
			}
			
			}//for
			m2.unlock();						
			m.unlock();

}

void run(serverstruct *d){//���� ��������� ���������� � ��������
	try{
		//���������� �������������� ���������
		if (access(("msgs-"+d->login+".dat").c_str(), F_OK) == 0) { 				// file exists?
			mf.lock();
			FILE *f=fopen((string("msgs-")+d->login+".dat").c_str(),"rb");
			try{
				while(true){
					if(feof(f)) break;
					string login0,msg0;
					//
						login0=readS(f);
						if(login0=="") break;
						msg0=readS(f);									
						m.lock();
						try{
							writeS(*(d->s),login0);	
							writeS(*(d->s),msg0);					
						}catch(...){m.unlock();throw tmpException();}

						m.unlock();					
					cout << "SEND Cached:" <<d->login <<' ' <<login0 <<' ' <<msg0 <<endl;				
				}
				fclose(f);
				remove(("msgs-"+d->login+".dat").c_str());
				mf.unlock();					
			}catch(...){
				fclose(f);					
				mf.unlock();										
				throw tmpException();
			}			
		}
		//-���� ��������� ���������
		while(true){		
			string cmd;	
			cmd=readS(*(d->s)); //"S" - �������� ��������� ������ ������������
			if(cmd=="S"){
				string LOGIN=readS(*(d->s));
				string msg=readS(*(d->s));
				sendMsg(d,msg,LOGIN);				
			}else if(cmd=="A"){ //"S" - �������� ��������� ���� �������������
				string msg=readS(*(d->s));			
				sendMsgAll(d,msg);
			}
		}					
	}catch(...){ //��������� ���������� � ������� ������ ����
		cout << "!run() ex!" <<endl;
		{
			m2.lock();
			try{
				d->s->close();//��������� �����
			}catch(...){}
			delete d->s; //������� ������ ������
			d->s=nullptr;
			d->t->detach();//����������� ���� �� ������� ����
			delete d->t; //������� ������ ����
			d->t=nullptr;
			m2.unlock();			
		}
		cout << "!run() ex-E!" <<endl;
		//break; //������� �� �����, �������� ����
	}
}


	void chatserver::run1(){
		setlocale (LC_ALL,"Russian");
	
		//������ ����� �� �������������
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
			if(command=="A"){//������� ����������� �������
				//����������� �������
				string login,pass;
				try{
					login=readS(*s);		
					pass=readS(*s);				
				}catch(...){
					delete s;
					continue;					
				}
				m.lock(); m2.lock();
				bool b=true,b1=true;// b-���� ����� ������������, b1-��������� ����� � ������ � ��� �����������
				try{
					if(data1.find(login)==data1.end()) b=false; //(��� �������� � ����� ��������?)
					else{
						if(data1[login]!=nullptr && data1[login]->login==login && data1[login]->passw==pass && data1[login]->t==nullptr){
						}else b1=false;
					}
				}catch(...){//���������� - ��� ������ ������������
					b=false;
				}
				if(b && b1){
					try{
						writeS(*s,"ok"); //����������� �������						
						//�������� ������ �������������
						for(auto ms: data1){
							writeS(*s,ms.second->login);
							writeS(*s,ms.second->fio);				
						}
						writeS(*s,"");			//(������ ������ �������� ����� ������)						
						data1[login]->setS(s);						
					}catch(...){
						m.unlock();	 m2.unlock();					
						s->close();
						delete s;
						continue;						
					}
				}else{
					try{
						writeS(*s,"err");  //����������� ���������
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
			}else if(command=="R"){//�����������
				string login,pass,fio;			
				bool b=true,b1=true;  //(b-��� ������� ������, b1-��� ������������ � ����� ������)
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
				if(b && b1){//�����������
					//
					remove("serverdb1.dat");	
					CopyFile(//��������� ��������� ����� ����� ��� ��������������
					  	"serverdb.dat",
  						"serverdb1.dat",
  						false
					);
					FILE *f=fopen("serverdb1.dat","ab");
					writeS(f,login);
					writeS(f,pass);			
					writeS(f,fio);			
					fclose(f);
					CopyFile(//�������� ��������� ������ �������� ����
					  	"serverdb1.dat",
  						"serverdb.dat",
  						false
					);					
					//
					try{
						writeS(*s,"ok");
					}catch(...){}
					s->close();	
					delete s;									
					//
					//this->done(); //��������� ������		
					m.lock();
					data1.insert(make_pair(login,new serverstruct(fio,login,pass)));
					m.unlock();
				}else{
					try{
						writeS(*s,"err");
					}catch(...){}	
					delete s;									
					//					
				}

			}else{//����������� �������
				delete s;
			}
		}
	}
	chatserver::done(){
		cout << "~chatserver()-B" << endl;
		m.lock();
		for(auto i:data1) if(i.second!=nullptr){
			if(i.second->s!=nullptr) {// ��������� �����(�������� � ���������� ����)
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


void t1(){//���� ������ ���������� ������ ��� �������
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

