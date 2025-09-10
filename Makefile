default:
	g++ -o main.exe main.cpp  -I include -L lib -lraylib -lgdi32 -lwinmm
	./main.exe
