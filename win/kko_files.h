
//windows version:

#pragma once
#include <string>
#include <unistd.h>
#include <winbase.h>

using namespace std;

bool fExists(string path){
	return access(path.c_str(), F_OK) == 0;
}

void CopyFile(string src, string dest){
	CopyFile(//сохраняем временную копию файла для редактирования
		  	src.c_str(),
			dest.c_str(),
			false
		);		
}

/* */



