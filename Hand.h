#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

#ifndef HAND_H
#define HAND_H
class Hand{
	protected:
		int max_capacity; 	//total fingers or toes
		int digits; 		//fingers or toes held up
		bool living;		//dead or alive
		
	public:
		//default hand constructor, default # of digits held up is 1
		Hand(){
			max_capacity = 5;
			digits = 1;
			living = true;
		}

		//custom hand constructor with custom # of digits
		Hand(int numdigits, int capacity){
			max_capacity = capacity;
			digits = numdigits;
			update_status();
		}

		//set digits held up
		void set_digits(int x){
			digits = x;
		}

		//get digits held up
		int get_digits(){
			return digits;
		}

		//receive digits
		//apply roll over max_capacity rule; default for hand
		//update status after; if still living or dead
		void add_digits(int x){
			digits += x;
			if(digits > max_capacity){
				digits -= max_capacity;
			}

			update_status();
		}

		//set current status of hand
		void set_living(bool b){
			living = b;
		}

		//get current status of hand
		bool is_living(){
			update_status();
			return living;
		}

		//check if hand is still living or dead then update status
		void update_status(){
			
			//the way this is coded; it shouldn't go over max capacity.
			if(digits < max_capacity){
				living = true;
			}else{
				living = false;
			}
		}
};
#endif
