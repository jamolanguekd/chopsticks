#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "Hand.h"
#include "Foot.h"
#include "socketstream/socketstream.hh"

using namespace std;
using namespace swoope;

#ifndef PLAYER_H
#define PLAYER_H
class Player{
	protected:
		int player_number;	//player number
		int player_team_number; //team number of this player
		socketstream* socket;    //socket of this player
		string type;		//player "class"
		vector<Foot> feet;	//collection of feet 
		vector<Hand> hands; //collection of hands
		bool living;		//dead or alive
		bool skip;			//flag is player is going to be skipped
		int actions_left;	//number of actions left
		int MAX_ACTIONS;	//max actions per turn	
		int numhands = 0;
		int numfingers = 0;
		int numfeet = 0;
		int numtoes = 0;
		
	public:
		
		//default constructor; human
		Player(){
			
			player_number = 0;
			type = "HUMAN";
			living = true;
			skip = false;
			actions_left = 1;
			MAX_ACTIONS = 1;
			
			//default 2 hands
			for(int i = 0; i < 2; i++){
				hands.push_back(Hand());
			}
			
			//default 2 feet
			for(int i = 0; i < 2; i++){
				feet.push_back(Foot());
			}
		}

		//custom constructor
		Player(string stype, int pnumber, int ptnumber, socketstream* s){
			
			socket = s;
			player_number = pnumber;
			player_team_number = ptnumber;
			type = stype;			
			living = true;
			skip = false;
			actions_left = 1;
			MAX_ACTIONS = 1;	
			
			if(type == "HUMAN"){
				numhands = 2;
				numfingers = 5;
				numfeet = 2;
				numtoes=  5;
			} else if(type == "ALIEN"){
				numhands = 4;
				numfingers= 3;
				numfeet = 2;
				numtoes = 2;
			} else if(type == "ZOMBIE"){
				numhands = 1;
				numfingers = 4;
				actions_left = 2;
				MAX_ACTIONS = 2;			//ZOMBIE HAS 2 ACTIONS PER TURN
			} else if(type == "DOGGO"){
				numfeet = 4;
				numtoes = 4;
			}
			
			
			//create their hands
			for(int i = 0; i < numhands; i++){
				hands.push_back(Hand(1, numfingers));
			}
			
			//create their feet
			for(int i = 0; i < numfeet; i++){
				feet.push_back(Foot(1, numtoes));
			}
		}

		//get player number
		int get_player_number(){
			return player_number;
		}

		//get player team number
		int get_player_team_number(){
			return player_team_number;
		}

		//get socket
		socketstream* get_socket(){
			return socket;
		}
		
		//set type of player
		void set_type(string s){
			type = s;
		}
		
		//get type of player
		string get_type(){
			return type;
		}
		
		//set status of player
		void set_living(bool b){
			living = b;
		}
		
		//get status of player
		bool is_living(){
			update_living();
			return living;
		}
		
		//update status of player
		void update_living(){
			//if atleast 1 foot is alive, player is still alive;
			for(int i = 0; i < feet.size(); i++){
				if(feet[i].is_living()){
					living = true;
					return;
				}
			}
			//if atleast 1 hand is alive, player is still alive;
			for(int i = 0; i < hands.size(); i++){
				if(hands[i].is_living()){
					living = true;
					return;
				}
			}	
			living = false;
			return;
		}


		//this player attacks another player
		string attack(string attacking_part, Player &other_player, string defending_part){
				string status = "";

				//attacking hand #
				int attacking_number = toupper(attacking_part[1]) - 65;
				
				//defending hand #
				int defending_number = toupper(defending_part[1]) - 65;

				//ATTACK WITH HAND
				if(attacking_part[0] == 'H'){
					//EH for nonexistent weapon
					if(attacking_number >= hands.size()){
						status = "INVALID MOVE! You are trying to attack with something that does not exist. Please try again.";
						return status;
					}

					//ATTACK HAND WITH HAND
					if(defending_part[0] == 'H'){
						
						if(defending_number >= other_player.get_hands()->size()){
							status = "INVALID MOVE! The hand you're trying to attack does not exist. Please try again.";
							return status;
						}

						if(other_player.get_hands()->at(defending_number).is_living() == false){
							status = "INVALID MOVE! You may not attack a dead hand. Please try again.";
							return status;
						}

						//update fingers of defending hand
						(*other_player.get_hands())[defending_number].add_digits(hands[attacking_number].get_digits());		
						
						//if defending player is a zombie AND 1 lang ang kamay nila tapos nadeads un, regrow
						if(other_player.get_type() == "ZOMBIE"){
							if(defending_number == 0 && !(*other_player.get_hands())[defending_number].is_living() && (*other_player.get_hands()).size() == 1){
								other_player.regrow();
							}
						}
					} 
					
					//ATTACK FEET WITH HAND
					else if(defending_part[0] == 'F'){
						if(defending_number >= other_player.get_feet()->size()){
							status = "INVALID MOVE! The feet you're trying to attack does not exist. Please try again.";
							return status;
						}

						if(other_player.get_feet()->at(defending_number).is_living() == false){
							status = "INVALID MOVE! You may not attack a dead foot. Please try again.";
							return status;
						}

						//update toes of defending feet
						(*other_player.get_feet())[defending_number].add_digits(hands[attacking_number].get_digits());
						
						//if defending player is non-alien and their defending feet dies, they will earn a skip
						if(!(*other_player.get_feet())[defending_number].is_living() && other_player.get_type() != "ALIEN"){
							other_player.set_skip(true);
						}
					}

					else{
						status = "INVALID MOVE! You are trying to attack something that does not exist. Please try again.";
						return status;
					}
				} 
				
				//ATTACK WITH FEET
				else if(attacking_part[0] == 'F'){
					//EH for nonexistent feet
					if(attacking_number >= feet.size()){
						status = "INVALID MOVE! You are trying to attack with something that does not exist. Please try again.";
						return status;
					}

					//ATTACK HAND WITH FEET
					if(defending_part[0] == 'H'){
						
						if(defending_number >= other_player.get_hands()->size()){
							status = "INVALID MOVE! The hand you're trying to attack does not exist. Please try again.";
							return status;
						}

						if(other_player.get_hands()->at(defending_number).is_living() == false){
							status = "INVALID MOVE! You may not attack a dead hand. Please try again.";
							return status;
						}

						//update fingers of defending hand
						(*other_player.get_hands())[defending_number].add_digits(feet[attacking_number].get_digits());
						
						//if defending player is a zombie AND 1 lang ang kamay nila tapos nadeads un, regrow
						if(other_player.get_type() == "ZOMBIE"){
							if(defending_number == 0 && !(*other_player.get_hands())[defending_number].is_living() && (*other_player.get_hands()).size() == 1){
								other_player.regrow();
							}
						}
						
					} 
					
					//ATTACK FEET WITH FEET
					else if(defending_part[0] == 'F'){
						if(defending_number >= other_player.get_feet()->size()){
							status = "INVALID MOVE! The feet you're trying to attack does not exist. Please try again.";
							return status;
						}

						if(other_player.get_feet()->at(defending_number).is_living() == false){
							status = "INVALID MOVE! You may not attack a dead foot.";
							return status;
						}

						//update toes of defending feet
						(*other_player.get_feet())[defending_number].add_digits(feet[attacking_number].get_digits());
						
						//if defending player is non-alien and their defending feet dies, they will earn a skip
						if(!(*other_player.get_feet())[defending_number].is_living() && other_player.get_type() != "ALIEN"){
							other_player.set_skip(true);
						}
					}

					else{
						status = "INVALID MOVE! You are trying to attack something that does not exist.";
						return status;
					}
				} 

				else{
					status = "INVALID MOVE! You are trying to attack with something that does not exist.";
					return status;
				}
				
				//NON-DOGGOS ATTACKING DOGGOS WILL EARN SKIPS; DONT HURT GOOD BOI
				if(type != "DOGGO" && other_player.get_type() == "DOGGO"){
					skip = true;
				}
				
				//if this function is called, it means an action has been consumed
				actions_left--;

				status = "Player #" + to_string(player_number) + " used " + attacking_part + " to attack Player #" + to_string(other_player.get_player_number()) + "'s " + defending_part +".";
			        return status;	
		}

		int count_living_hands(){
			int count = 0;

			for(int i = 0; i < hands.size(); i++){
				if(hands[i].is_living()){
					count++;
				}
			}
			
			return count;
		}

		int count_living_feet(){
			int count = 0;
			
			for(int i = 0; i < feet.size(); i++){
				if(feet[i].is_living()){
					count++;
				}
			}
		
			return count;
		}

		bool validate_transfer_hands(vector<int> hand_values){

			//store hand status
			bool valid_hands[hands.size()];
			for(int i = 0; i < hands.size(); i++){
				if(hands[i].is_living()){
					valid_hands[i] = true;
				} else{
					valid_hands[i] = false;
				}
			}

			//check for equal redistribution
			int original_count = 0;
		        int new_count = 0;
			int j = 0;
			for(int i = 0; i < hands.size(); i++){
				if(valid_hands[i]){
					if(hand_values[j] == numfingers) return false;
					original_count += hands[i].get_digits();
					new_count += hand_values[j];
					j++;
				}
			}

			if(original_count != new_count) return false;

			//check for same matches
			j = 0;
			for(int i = 0; i < hands.size(); i++){
				if(valid_hands[i]){
					if(hand_values[j] != hands[i].get_digits()){
						return true;
					}
					j++;
				}
			}

			return false;
		}

		bool validate_transfer_feet(vector<int> feet_values){

			bool valid_feet[feet.size()];

			for(int i = 0; i < feet.size(); i++){
				if(feet[i].is_living()){
					valid_feet[i] = true;
				} else{
					valid_feet[i] = false;
				}
			}

			int original_count = 0;
		        int new_count = 0;
			int j = 0;
			for(int i = 0; i < feet.size(); i++){
				if(valid_feet[i]){
					if(feet_values[j] == numtoes) return false;
					original_count += feet[i].get_digits();
					new_count += feet_values[j];
					j++;
				}
			}
			
			if(original_count != new_count) return false;

			j = 0;
			for(int i = 0; i < feet.size(); i++){
				if(valid_feet[i]){
					if(feet_values[j] = feet[i].get_digits()){
						return true;
					}
					j++;
				}
			}

			return false;
		}


		//changing the finger values of your hands; disthand
		void transfer_hands(vector<int> hand_values){
			
			int j = 0;
			for(int i = 0; i < hands.size(); i++){
				if(hands[i].is_living()){
					hands[i].set_digits(hand_values[j]); //only edit hands that are still alive
					 
					j++;
				}
			}

			actions_left--;
		}
		
		//changing toe values of feet; distfeet
		void transfer_feet(vector<int> feet_values){
			
			int j = 0;
			for(int i = 0; i < feet.size(); i++){
				if(feet[i].is_living()){
					feet[i].set_digits(feet_values[j]);	//only edit feet that are still alive
				 	j++;
				}
			}
			
			actions_left--;
		}

		//get set of hands
		vector<Hand> *get_hands(){
			return &hands;
		}
		
		//get set of feet
		vector<Foot> *get_feet(){
			return &feet;
		}
		
		//get actions left
		int get_actions() {
			return actions_left;
		}
		
		//replenish actions
		void reset_actions(){
			actions_left = MAX_ACTIONS;
		}
		
		//get skip status
		bool get_skip(){
			return skip;
		}
		
		//set skip status
		void set_skip(bool b){
			skip = b;
		}
		
		//add a new hand; specified for zombie
		void regrow(){
			hands.push_back(Hand(1,4));
		}
		
		//ALLOWS U TO COMPARE PLAYER OBJECTS USING THEIR PLAYER NUMBER VALUE
		bool operator < (const Player& other_player) const
		{
		    return (player_number < other_player.player_number);
		}
};
#endif
