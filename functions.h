#ifndef FUNTIONS
#define FUNTIONS

#include <bits/stdc++.h>
using namespace std;


// Global Variables & data types :

extern long long int n, d;    // n = number of instances , d = number of attributes 
extern long long int train[1000][10001], test[1000][10001];  // train & test data sets
extern unordered_map<int,int> mapping;                    // mapping betn attibute indices & 0-4999
extern double splitting_threshold, info_gain_threshold, global_error ; 
extern long long int max_height ;                         // max depth of tree
extern map<long long int, long long int> attribute_list ;    
extern long long int attrCount[10000];


// Function declarations :

void addNoise(double p);



#endif /* FUNTIONS */