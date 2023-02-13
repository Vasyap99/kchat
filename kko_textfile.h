namespace kko{

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

void writeS(FILE*f,const string &s){
	fputs((s+"\r\n").c_str(),f);
}
void writeS(FILE*f,const string &&s){
	fputs((s+"\r\n").c_str(),f);
}
	
}
