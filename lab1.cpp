#include <bits/stdc++.h>
#include "functions.h"

using namespace std;


// Variables & data types
long long int n=1000, d=10000;    // n = number of instances , d = number of attributes 
long long int train[1000][10001], test[1000][10001];  // train & test data sets
unordered_map<int,int> mapping;                    // mapping betn attibute indices & 0-9999
unordered_map<int,int> mapping_reverse; 
double splitting_threshold, info_gain_threshold, global_error ; 
long long int max_height ;                         // max depth of tree
map<long long int, long long int> attribute_list ;    
long long int attrCount[10000];


// Comparison Operator
bool comp(pair<int,double> a, pair<int,double> b) 
{
    return a.second < b.second;
}


// node structure
typedef struct Node
{	
	map<long long int, Node*> child_set ; // map for storing childs
	Node *parent ;
	int attribute, plus, minus, current, label ; 
	vector<int> instances ;  // set of instances at that node

	Node()  
	{
		child_set.clear() ;  
		plus = 0 ;
		minus = 0 ;  
		attribute = -1 ;  
		label = -1 ;  
	}
}node; 

node* head;


// Prototype for functions
int read_data(string str);
int read_test_data(string str);
void Preprocessing();
int decide(int index, node *curr);
int decide_for_training(int index, node *curr);
vector <node*> get_child_vector(map<long long int,node*> m);
int initialise();
int set_head(node* head);
double calculate_error();
long long int countNodes(node* root);
//void addNoise(double p);
int id3(node *curr, int h);
int ensemble(long long int num);
double calculate_entropy(int p,int m);
long long int choose_best_attribute(vector<int> a, double root_entropy);



// function to create Random Forest :
int ensemble(long long int num)
{
	node *forest[num] ; 
	long long int i, j ,k, plus_count, minus_count, result = -1;
	vector <long long int> attribute_shuffle;
	double test_error = 0, train_error = 0 ;
	
	for(i=0;i<d;i++)
	{
		attribute_shuffle.push_back(i) ;
	}
	
	for(k=0;k<num;k++)
	{
		attribute_list.clear() ; 
	
		forest[k] = new node() ;	
		forest[k]->parent = NULL ;
		
		// randomize the attributes :	
		random_shuffle(attribute_shuffle.begin(), attribute_shuffle.end()) ;	
		for(i=5000;i<d;i++)
		{
			attribute_list[attribute_shuffle[i]] = 100 ; 
		}
		
		set_head(forest[k]) ; 
		id3(forest[k], 0) ;
	
	}

	for(i=0;i<1000;i++)
	{
		plus_count = minus_count = 0 ;
		for(k=0;k<num;k++)
		{
			result = decide(i, forest[k]) ;
		
			if(result == 1)
				plus_count++ ;
			else
				minus_count++ ;
		}
		
		result = (plus_count > minus_count)? 1 : 0;
		
		if(result != test[i][d])
			test_error++ ;
	}
	
	for(i=0;i<1000;i++)
	{
		plus_count = minus_count = 0 ;
		for(k=0;k<num;k++)
		{
			result = decide_for_training(i, forest[k]) ;
		
			if(result == 1)
				plus_count++ ;
			else
				minus_count++ ;
		}
		
		result = (plus_count > minus_count) ? 1 : 0;
		
		if(result != train[i][d])
			train_error++ ;
	}
	
	test_error = test_error / (double) (1000) ;
	test_error = 100 * (1 - test_error) ;
	
	train_error = train_error / (double) (1000) ;
	train_error = 100 * (1 - train_error) ;
		
	cout << setw(20) << train_error << setw(20) << test_error << endl ;  
	
}


// Setting head node with instances
int set_head(node* head)
{
	long long int i ; 	
	max_height = 0 ;
	for(i=0;i<1000;i++)
	{
		head->instances.push_back(i) ; 
	}
}


// Recursive function for splitting nodes
int id3(node *curr, int h)
{

	double root_entropy, pplus, pminus, purity_plus, purity_minus;
	long long int i, total = curr->instances.size() ;
	
	for(i=0;i<total;i++)
	{
		if(train[curr->instances[i]][d] == 1)
			curr->plus++ ;
		else
			curr->minus++ ; 
	}

	if(max_height < h)
		max_height = h ;                   // Updating max height
	
	// Calculating entropy
	
	root_entropy = calculate_entropy(curr->plus,curr->minus); 	
	
	// Early Stopping Condition 

	purity_plus = curr->plus / (double) total ;  
	
	if(purity_plus >= splitting_threshold) 
	{
		curr->label = 1 ;
		return 0 ;
	}
	
	purity_minus = curr->minus / (double) total ; 
	
	if(purity_minus >= splitting_threshold) 
	{
		curr->label = 0 ;
		return 0 ;
	}
	
	// End of early stopping condition  
	
	long long int attribute = choose_best_attribute(curr->instances, root_entropy) ;        //  Returns best attribute to pick
		
	
	if(attribute == -4)					                     //  Zero Info Gain
	{
		curr->label = train[curr->instances[0]][d] ;
		return 0 ;  
	}
	
	if(attribute == -5)                                       // All attributes over
	{
		curr->label = (curr->plus > curr->minus) ? 1 : 0;		
		return 0 ;
	}
	
	
	curr->attribute = attribute ;	
	attribute_list[attribute]++ ;                                // Attribute is used up 	
	
	for(i=0;i<total;i++)
	{
		long long int element = curr->instances[i] ; 
		long long int key = train[element][attribute] ;  
	
		if(curr->child_set[key] == NULL)
		{
			curr->child_set[key] = new node() ; 
			curr->child_set[key]->current = key ; 
			curr->child_set[key]->parent = curr ;            
			curr->child_set[key]->instances.push_back(element) ; 
		}
		else
		{
			curr->child_set[key]->instances.push_back(element) ; 
		}
	}
		
	for(auto it=curr->child_set.begin(); it!=curr->child_set.end(); it++)
    {
   		id3(it->second, h+1) ;  	
    }
	
	attribute_list[attribute]-- ;                     // Attribute is freed up 
	
	return 0 ; 

}


// Function to decide the label for test data
int decide(int index, node *curr)
{
	
	long long int attribute = curr->attribute ; 
	int ans;

	if(curr->label != -1)
		return curr->label ; 

	if(curr->child_set[test[index][attribute]] != NULL)
	{
		return decide(index, curr->child_set[test[index][attribute]]) ;
	}
	else
	{
		ans = (curr->plus > curr->minus)? 1 : 0;
		return ans ;
	}
	
}


// Function to decide the label for training data
int decide_for_training(int index, node *curr)
{
	
	long long int attribute = curr->attribute ; 
	int ans;

	if(curr->label != -1)
		return curr->label ; 

	if(curr->child_set[train[index][attribute]] != NULL)
	{
		return decide_for_training(index, curr->child_set[train[index][attribute]]) ;
	}
	else
	{
		ans = (curr->plus > curr->minus)? 1 : 0;
		return ans ;
	}
	
}


// Function to find the best attribute among available attributes
long long int choose_best_attribute(vector<int> a, double root_entropy)
{ 
	double maximum = 0, info_gain, loss, temp ;
	long long int final_attribute = -1, attribute, total = a.size(),  i, j ;
	map<pair<long long int, long long int> , long long int> classifier ;  // classifier map
 	
	for(attribute=0;attribute<d;attribute++) 
	{
		if(attribute_list[attribute] > 0)
			continue ;  
	
		classifier.clear() ;
		loss = 0 ;
	
		for(i=0;i<total;i++)
		{
			classifier[make_pair(train[a[i]][attribute],train[a[i]][d])]++ ; 
		}
		
		map<pair<long long int, long long int>, long long int>::iterator it, next ;
		
		for(it=classifier.begin(); it!=classifier.end();)
    	{
    		
    		double entropy, pplus, pminus, pall ;
    		
    		next = ++it ;
    		it-- ;
    		
    		if(next == classifier.end())
    		{
    			entropy = 0 ; 
    			pall = it->second ; 
    			it++ ;
    		}
    		else
    		{
    			if(it->first.first == next->first.first)
    			{
					entropy = 0 ;
					
					pminus = it->second ;
					pplus = (next)->second ; 
					pall = pplus + pminus ; 
					entropy = calculate_entropy((int) pplus,(int)pminus) ;
					
					it++ ;
					it++ ; 
    			}
    			else
    			{    			
    				entropy = 0 ;
    				pall = it->second ;  
    				it++ ;
    			}
    		}
    		
    		temp = pall / (double)total ;
    		
    		temp = temp * entropy ;
    		loss = loss + temp ;
    	}
    	
    	info_gain = root_entropy - loss ;       
    	
    	if(info_gain >= maximum)
    	{
    		maximum = info_gain ;
    		final_attribute = attribute ;  
    	} 
	}
	
	if(maximum < info_gain_threshold)
		final_attribute = -4 ; 
		
	if(final_attribute == -1)
		final_attribute = -5 ;
	
	return final_attribute ; 
	
}


// Function to calculate Entropy
double calculate_entropy(int p,int m)
{
	double plus,minus;
	long long int total = p + m;
	double result;

	plus = (double) p / (double) total ;
	minus = (double) m / (double) total ;

	result = - (plus * log2(plus)) - (minus * log2(minus)) ;

	return result;
}


// Function to get child vector 
vector <node*> get_child_vector(map<long long int,node*> m)
{

   	vector <node*> ch;
	for (auto it=m.begin(); it!=m.end();it++)
	{
	    if ( (it->second) !=NULL)
			ch.push_back(it->second);
	}
	return ch;
} 


// Setting up of solution
int initialise()
{
	splitting_threshold = 1 ;
	max_height = 0 ;
	info_gain_threshold = 0.0000001 ;
	global_error = 1000 ;  
	attribute_list.clear() ; 
}


// Function to calculate error on test data of 1000 instances
double calculate_error()
{
	long long int result = 0, i ;
	double error = 0 ;
	for(i=0;i<1000;i++)
	{
		result = decide(i, head) ;
		if(result != test[i][d])
			error++ ;
	}

	// Get error on test data of 1000 elements
	error = error / (double) (1000) ;
	return error ; 
}


// Function to count nodes of a sub-tree
long long int countNodes(node* root)
{
	long long int cont = 0, i;
 	queue< node* > q;                    // Level Order Traversal
 	vector < node* > vec;
     
    if (root==NULL)
   		return 0 ;
   		
  	q.push(root);
	while(!q.empty())
	{
		  node* fnt=q.front();
		  attrCount[ fnt->attribute]++;        // increasing count for visited attribute
		  cont++;
		  q.pop();		  
		  vec = get_child_vector(fnt->child_set);		  
		  for (i=0;i<vec.size();i++)
		  { 
			 if ((vec[i]) != NULL)
			 	q.push(vec[i]) ; 	 
		  }
	}

return cont;
} 

// Prunning function to print when their is increase in accuracy due to node removal
int pruning(node *curr)
{	
	if(curr == NULL)
		return 0 ;

	long long int i, total = curr->child_set.size() ;
	double local_error ;
	node *temp ; 

	if(total == 0)
		return 0 ; 
	
	map<long long int,node*>::iterator it;
 
	for (it=curr->child_set.begin(); it!=curr->child_set.end();it++)
	{
	    pruning(it->second) ;
	}
	
	for (it=curr->child_set.begin(); it!=curr->child_set.end();it++)
	{
	    temp = it->second ; 
	    it->second = NULL ; 
	    local_error = calculate_error() ;
	    
	    if(local_error < global_error)
	    {
	    	global_error = local_error ; 
	    	cout << setw(8) << (100 - global_error*100) << setw(16) << countNodes(head) << endl ;
	    }
	    else
	    {
	    	it->second = temp ;	
	  	} 
	}
	
}



// Filling data values in train array :
int read_data(string str)
{

	ifstream in2(str);

	long long int i, j,ct=0,p,index;
	string line;
	
	for(i=0;i<1000;i++)
	{
		for(j=0;j<=10000;j++)
		{
			train[i][j] = 0; 
		}
	}

	unordered_map<int,int>:: iterator it;
	vector <string> tokens;
	string intermediate;
	char* token,*C;

	if (in2.is_open())
	 {
		while (getline(in2, line))
		 {
     
    		// Tokenizing w.r.t. space ' :'
			C = new char[line.size()+1];
			strcpy(C, line.c_str());
    		token = strtok(C," :");
 
  	  		// Keep printing tokens while one of the
    		// delimiters present in str[].
    		while (token != NULL)
    		{
        		tokens.push_back(token);
    		    token = strtok(NULL, " :");
   			}

            if(atoi(tokens[0].c_str()) >= 7)
				train[ct][10000] = 1;
			else
				train[ct][10000] = 0;
			for(int i = 1; i < tokens.size(); i=i+2)
			{
       			if(i % 2 != 0)
       			{					
					p = atoi(tokens[i].c_str());
					if(mapping.find(p) != mapping.end())
					{
						it = mapping.find(p);
						index = it->second;
						train[ct][index] = atoi(tokens[i+1].c_str());
					}

				}

			}
			ct++;
			tokens.clear();

		}
	}    
	in2.close();

} 


// Filling data values in test array :
int read_test_data(string str)
{
	ifstream in2(str);

	long long int i, j,ct=0,p,index;
	string line;
	
	for(i=0;i<1000;i++)
	{
		for(j=0;j<=10000;j++)
		{
			test[i][j] = 0; 
		}
	}

	unordered_map<int,int>:: iterator it;
	vector <string> tokens;
	string intermediate;
	char* token,*C;

	if (in2.is_open())
	 {
		while (getline(in2, line)) 
		{
     
    		// Tokenizing w.r.t. space ' :'
			C = new char[line.size()+1];
			strcpy(C, line.c_str());
    		token = strtok(C," :");
 
  	  		// Keep printing tokens while one of the
    		// delimiters present in str[].
    		while (token != NULL)
    		{
        		tokens.push_back(token);
    		    token = strtok(NULL, " :");
   			}

            if(atoi(tokens[0].c_str()) >= 7)
				test[ct][10000] = 1;
			else
				test[ct][10000] = 0;
			for(int i = 1; i < tokens.size(); i=i+2)
			{
       			if(i % 2 != 0)
       			{					
					p = atoi(tokens[i].c_str());
					if(mapping.find(p) != mapping.end())
					{
						it = mapping.find(p);
						index = it->second;
						test[ct][index] = atoi(tokens[i+1].c_str());
					}

				}

			}
			ct++;
			tokens.clear();

		}
	}    
	in2.close();

} 

// Preprocessing done before moving to the actual code  
void Preprocessing()
{

// Selecting 5000 features on basis of their ratings

	unordered_map<int,double> mp;
	int ct=1,i=0;
	double a;
	ifstream in("imdbEr.txt");
	ofstream out1;
	out1.open ("selected-features-indices.txt");
	if (in.is_open()) 
	{	
		while(in >> a)
		{
			mp[ct] = a;
			ct++;
		}

	}

	vector<pair<int,double>> elems(mp.begin(), mp.end());
	sort(elems.begin(), elems.end(), comp);

	for(auto it = elems.begin();it != elems.end(),i<5000;it++,i++)
	{
		out1 << it->first << "\n";
	}
	reverse(elems.begin(), elems.end());
	for(auto it = elems.begin();it != elems.end(),i<10000;it++,i++)
	{
		out1 << it->first << "\n";
	}

	in.close();
	out1.close();

	// creating mapping for indices vs 0-9999 bound :

	ct=0;
	i=0;
	ifstream in2("selected-features-indices.txt");
	if (in2.is_open())
	{
		while(in2 >> i)
		{
			mapping[i] = ct;
			mapping_reverse[ct] = i;
			ct++;
		}

	}
	in2.close();
}


int main(int argc,char** argv)
{

int i,j;
if(argc != 4)
{
	cout << "4 arguments are expected , Try again !!!";
}
else
{
	string training_file = argv[1];
	string test_file = argv[2];
	char c = argv[3][0];
	double r1,r2;
	if(c == '2')
	{
		Preprocessing();
		read_data(training_file);
		read_test_data(test_file);
		initialise();
		cout << setw(40) << "Experiment no. - 2\n\n" ;
		cout << "Early Stopping Threshold %" << setw(26) << "Test accuracy %" << setw(16) << "No. of nodes" << endl << endl ;
		for(double iter=0.94;iter<1;iter=iter+0.01)
		{
			for (int i=0;i<d;i++)
			{
				attrCount[i]=0;
			}

			splitting_threshold = iter ;
			head = new node() ;	
			head->parent = NULL ; 
			set_head(head) ;
			id3(head, 0) ;
			int nnodes = countNodes(head) ;
			r1 = calculate_error(); 
			r2 = nnodes;	
			cout << setw(4) << (iter*100) << setw(40) << (100-r1*100) << setw(16) << (int)r2 << endl ; 

		}
		cout << "\nNumber of times an attribute is used as the splitting function : " << endl << endl ;		
		vector < pair<int,int> > afreq;	
		for (i=0;i<d;i++)
		{
	  		afreq.push_back(make_pair(attrCount[i],i));
		}
   		sort(afreq.begin(),afreq.end());
	  	int ans;
  		for (i=afreq.size()-1;i>=0;i--)
   		{
       		if (afreq[i].first>0)
       		{
       			ans = mapping_reverse[afreq[i].second];
	 	   		cout << setw(4) <<  ans<<" -> "<<afreq[i].first<<endl;
       		}
		}

	}

	else if(c == '3')
	{
		cout << setw(30) << "Experiment no. - 3\n\n" ;
		double noise[] = {0.5, 5, 10, 25} ;
		cout << "Noise %" << setw(16) << "Test accuracy" << setw(16) << "No. of nodes" << endl << endl ;
		Preprocessing();
		for(int iter=0;iter<4;iter++)
		{
			read_data(training_file);
			read_test_data(test_file);
			initialise() ;
			addNoise(noise[iter]) ;
			head = new node() ;	
			head->parent = NULL ; 
			set_head(head) ; 
			id3(head, 0) ;
			int nnodes = countNodes(head) ;
			r1 = calculate_error() ; 
			r2 = nnodes;	
			cout << setw(4) << noise[iter] << setw(16) << (100-r1*100) << setw(16) << (int)r2 <<endl ;	

		}


	}

	else if(c == '4')
	{
		Preprocessing();
		read_data(training_file);
		read_test_data(test_file);
		initialise();
		cout << setw(24) << "Experiment no. - 4\n\n" ;
		cout << "Test accuracy" << setw(16) << "No. of nodes" << endl << endl ;
		head = new node() ;	
		head->parent = NULL ;
		set_head(head) ; 
		id3(head, 0) ;	
		global_error = calculate_error() ;
		pruning(head) ;
		cout << endl ; 
	
	}

	else if(c == '5')
	{
		Preprocessing();
		read_data(training_file);
		read_test_data(test_file);
		initialise();
		cout << setw(34) << "Experiment no. - 5\n\n" ;
		cout <<"No. of trees" << setw(16) << "   Training accuracy" << setw(16) << "Test accuracy" << endl << endl ;
		for(i=1;i<=20;i+=1)
		{
			cout << setw(4) << i ;
			ensemble(i) ;
		}
	}
	else
	{
		cout << "Enter the expected arguments !!! "<<endl;
	}

}

return 0;

}

