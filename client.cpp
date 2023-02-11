
#include "kkoSocket.h"

using namespace kko;

int main(int argc, char** argv) {
	Socket s(Connect("127.0.0.1",8087));
	s.send("hello2");
	std::cout << s.recv(8);
	system("pause");
	return 0;
}
