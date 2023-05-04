p = pthread

chat: copyLin chatclient.cpp chatserver.cpp
	g++ -pthread chatclient.cpp -o chatclient
	g++ -pthread chatserver.cpp -o chatserver

copyLin:
	cp ./lin/* .

clean:
	rm -rf *.o chatclient chatserver chatserver.exe chatclient.exe

install: chatclient chatserver
	mkdir c1 c2 ; cp -p chatclient c1 ;	cp -p chatclient c2
