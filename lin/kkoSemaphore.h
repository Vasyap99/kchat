
extern "C"{
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <semaphore.h>
	#include <stdio.h>
	#include <string>
}

#define SEMAPHORE_NAME "/my_named_semaphore"


namespace kko{
    static int nnn;
	class Semaphore{
		int max;
		//HANDLE h;
		sem_t *sem;

		
	public:
		Semaphore(int max=1)
			:max(max)
		{
			nnn++;
			sem = sem_open( (string(SEMAPHORE_NAME)+std::to_string(nnn)).c_str(), O_CREAT, 0777, 0);
			sem_init(sem, 0, 1);
		}
		void lock(){
			sem_wait(sem);
		}
		void unlock(){
			sem_post(sem);
		}
		~Semaphore(){
			sem_destroy(sem);
		}
	};
}
