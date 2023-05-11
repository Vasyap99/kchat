#pragma once

#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <unistd.h>

#include <iostream> 
#include <sys/utsname.h> // Для uname()
using namespace std;

/*struct utsname {
  char sysname[];  // имя операционной системы 
  char nodename[]; // имя узла сети 
  char release[];  // номер выпуска операционной системы 
  char version[];  // номер версии этого выпуска 
  char machine[];  // тип аппаратной архитектуры 
};*/

void print_sysinfo() {
        struct utsname utsname; // объект для структуры типа utsname

        uname(&utsname); // передаем объект по ссылке

        // распечатаем на экране информацию об операционной системе
        // эту информацию нам вернуло ядро Linux в объекте utsname
        cout << "OS name: " << utsname.sysname << endl; 
        cout << "Host name: " << utsname.nodename << endl; 
        cout << "OS release: " << utsname.release << endl; 
        cout << "OS version: " << utsname.version << endl; 
        cout << "Architecture: " << utsname.machine << endl; 

		cout << endl;

		cout << getgid() << " идентификатор действительной группы текущего процесса" <<endl;
		cout << getpid() << " идентификатор текущего процесса" <<endl;
		cout << getpriority(PRIO_PROCESS,0) << " приоритет процесса" <<endl;
		cout << getuid() << " идентификатор пользователя процесса" <<endl;

}
