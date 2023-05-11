
#include <fstream>
#include <string>

using namespace std;

bool fExists(string path){
	ifstream my_file(path);
	if (my_file.good()){
		return true;
	}else return false;
}


void CopyFile(string src, string dest){
	system( ("cp "+src+" "+dest).c_str() );		
}


//windows version:
/*

#include <unistd.h>
#include <winbase.h>

bool fExists(string path){
	return access(("msgs-"+d->login+".dat").c_str(), F_OK) == 0;
}

void CopyFile(string src, string dest){
	CopyFile(//сохраняем временную копию файла для редактирования
		  	src.c_str(),
			dest.c_str(),
			false
		);		
}

*/



