
#include "kkoSocket.h"

using namespace kko;

int main(int argc, char** argv) {
	ServerSocket ss;
    ss.bind();
	while(true){
		ss.listen();
		Socket s=ss.accept();
		std::cout << s.recv(6);
		s.send("<string>");
	}
	return 0;
}
