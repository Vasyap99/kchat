#include <algorithm>

namespace kpy{
	#include <cctype>
	
	std::vector <std::string> * split(std::string s,char c){
		std::vector <std::string> * v=new std::vector <std::string>;
		std::string buf="";
		//o<<s.size()<<s<<"s.len\n";
		for(int i=0;i<s.length();i++){
			if(s[i]!=c){
				buf+=s[i];
			}else{
				v->push_back(buf);
				buf="";
			}
		}
		v->push_back(buf);
		return v;
	}
	std::vector <std::string> * split(std::string s,std::string c){
		std::vector <std::string> * v=new std::vector <std::string>;
		std::string buf="", buf1="";  int j=-1;		//o<<"splB_"<<s<<'_'<<c<<'_'<<"\r\n";
		//o<<s.size()<<s<<"s.len\n";
		for(int i=0;i<s.length();i++){				//o<<"spl0_"<<i<<"\r\n";
			if(j+1<c.length() && s[i]==c[j+1]){
				j+=1;
				buf1+=s[i];							//o<<"spl1_"<<buf<<'_'<<buf1<<']'<<"\r\n";
			}else if(j==-1){
				buf+=s[i];							//o<<"spl2_"<<buf<<'_'<<buf1<<']'<<"\r\n";
			}else if(j!=-1){
				if(j+1!=c.length()){
					buf+=buf1;
					buf1="";
					j=-1;							//o<<"spl3.1_"<<buf<<'_'<<buf1<<']'<<"\r\n";
					if(j+1<c.length() && s[i]==c[j+1]){
						j+=1;
						buf1+=s[i];							//o<<"spl3.1.1_"<<buf<<'_'<<buf1<<']'<<"\r\n";
					}
				}else{
					v->push_back(buf);
					buf="";
					buf1="";
					j=-1;							//o<<"spl3.2_"<<buf<<'_'<<buf1<<']'<<"\r\n";
					buf+=s[i];
				}
			}
		}
		if(buf!="")
		    v->push_back(buf);							//o<<"splE_"<<buf<<' '<<buf1<<"\r\n";
		return v;
	}	
	//template <typename T>
	//T sliceB()
	std::string sliceB(std::string s,int en){
		std::string buf;		
		for(int i=0;i<s.size()-(s.size()-en);i++){
			buf+=s[i];			
		}
		return buf;
	}
	std::string sliceE(std::string s,int en){
		std::string buf;		
		for(int i=en; i<s.size(); i++){
			buf+=s[i];			
		}
		return buf;
	}
	std::string slice(std::string s,int b,int e){
		std::string buf;		
		for(int i=b; i<e && i<s.length(); i++){
			buf+=s[i];			
		}
		return buf;
	}		
	
	bool strip__last_nonSpace_ch(std::string s,int i){
		bool b=true;
		for(int j=i;j<s.length();j++){
			if( !(s[j]==' ' || s[j]=='\r' || s[j]=='\n' || s[j]=='\t') ){
				b=false;
			}
		}
		return b;
	}
	std::string strip(std::string s){
		std::string buf;		
		int i=0;
		while(i<s.length() && (s[i]=='\r' || s[i]=='\n' || s[i]=='\t' ||s[i]==' ')){
			i++;
		}
		while(i<s.length()){
			if(strip__last_nonSpace_ch(s,i))
				break;
			buf+=s[i];
			i++;
		}
		return buf;
	}		
	

	/*
	int filefunc(){
    	WIN32_FIND_DATA FindFileData;
    	HANDLE hf;
    	hf=FindFirstFile("c:\\*", &FindFileData);
    	if (hf!=INVALID_HANDLE_VALUE){
        	do{
            	std::cout << FindFileData.cFileName << "\n";
        	}
        	while (FindNextFile(hf,&FindFileData)!=0);
        	FindClose(hf);
    	}
	}*/
	std::string lower(std::string s){
		std::string s1=s;
		std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
		return s1;
	}
}
