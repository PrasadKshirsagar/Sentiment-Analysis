
output : lab1.o exp_3.o
	g++ lab1.o exp_3.o -o output

lab1.o : lab1.cpp functions.h
	g++ -c lab1.cpp

exp_3.o : exp_3.cpp functions.h
	g++ -c exp_3.cpp 

clean : 
	rm*.o output	
