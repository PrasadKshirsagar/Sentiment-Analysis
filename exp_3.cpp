#include <bits/stdc++.h>
#include "functions.h"



using namespace std;


// Function to add noise into train data by randomly choosing instance indexes
void addNoise(double p)
{
	int temp,R;
	R = (int)(p*n/100);
	for (int i=0;i<R;i++)
	{	   
		int a=rand()%n;
		temp=train[a][d];
		train[a][d]= (train[a][d] == 0)? 1 : 0;
		
	}

}
