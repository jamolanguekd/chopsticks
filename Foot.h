#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "Hand.h"

using namespace std;

#ifndef FOOT_H
#define FOOT_H
class Foot : public Hand{
	
	public: 
		//default foot constructor, default # of digits held up is 1
		Foot(){
			max_capacity = 5;
			digits = 1;
			living = true;
		}
	
		//custom hand constructor with custom # of digits
		Foot(int numdigits, int capacity){
			max_capacity = capacity;
			digits = numdigits;
			update_status();
		}
		
		//remove the roll over restriction
		void add_digits(int x){
			digits += x;
			update_status();
		}
	
};
#endif
