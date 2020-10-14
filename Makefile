make clean:
	rm *.o quash

make quash:
	g++ -std=c++11 main.cpp -o quash