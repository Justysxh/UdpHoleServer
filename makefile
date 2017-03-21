objects= holesrv.o NetApiDefine.o SocketUDP.o UtilMakePack.o ThreadUtil.o

holesrv : $(objects)
	g++  $(objects) -lpthread -o holesrv
holesrv.o : HoleServer.cpp
	g++ -c HoleServer.cpp -o holesrv.o
NetApiDefine.o : NetApiDefine.cpp
	g++ -c  NetApiDefine.cpp -o NetApiDefine.o
SocketUDP.o : SocketUDP.cpp
	g++ -c SocketUDP.cpp -o SocketUDP.o
ThreadUtil.o : ThreadSyncUtil.cpp
	g++ -c ThreadSyncUtil.cpp -o ThreadUtil.o
UtilMakePack.o : UtilMakePack.cpp
	g++ -c UtilMakePack.cpp -o UtilMakePack.o
clean : 
	rm holesrv $(objects)
