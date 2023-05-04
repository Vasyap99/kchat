
extern "C"{
	#include <synchapi.h>
}


namespace kko{
	class Semaphore{
		int max;
		HANDLE h;
	public:
		Semaphore(int max=1)
			:max(max)
		{
			h=CreateSemaphore( 
	    		NULL,	// ??? ????????
	    		max,	// ????????? ?????????
	    		max,	// ???????????? ?????????
	    		NULL	// ??? ?????
			);
		}
		void lock(){
			WaitForSingleObject(h,INFINITE);
		}
		void unlock(){
			ReleaseSemaphore(h,1,NULL);
		}
		~Semaphore(){
			CloseHandle(h);
		}
	};
}
