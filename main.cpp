// PAIR WORK BY:
// Cruel, Nathaniel
// Jamolangue, Kyle

/*
some notes:

assuming no input validation is needed.
assuming input is consistent with game rules.
in other words, try to break the program within the game's rules.

-CANT ATTACK WITH DEAD HANDS OR FEET
-CANT ATTACK DEAD HANDS OR FEET
-CANT TRANSFER FINGERS/TOES TO DEAD HANDS/FEET
-CANT TRANSFER TOES KUNG WALANG FEET
-CANT TRANSFER FINGERS KUNG WALANG HAND
-NO FRIENDLY FIRE
-YOU CAN'T ATTACK YOURSELF

*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

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


class Player{
	protected:
		int player_number;	//player number
		string type;		//player "class"
		vector<Foot> feet;	//collection of feet 
		vector<Hand> hands; //collection of hands
		bool living;		//dead or alive
		bool skip;			//flag is player is going to be skipped
		int actions_left;	//number of actions left
		int MAX_ACTIONS;	//max actions per turn
		
	public:
		
		//default constructor; human
		Player(){
			
			player_number = 0;
			type = "human";
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
		Player(string stype, int pnumber){
			
			player_number = pnumber;
			type = stype;			
			living = true;
			skip = false;
			actions_left = 1;
			MAX_ACTIONS = 1;
			
			int numhands = 0;
			int numfingers = 0;
			int numfeet = 0;
			int numtoes = 0;
			
			if(type == "human"){
				numhands = 2;
				numfingers = 5;
				numfeet = 2;
				numtoes=  5;
			} else if(type == "alien"){
				numhands = 4;
				numfingers= 3;
				numfeet = 2;
				numtoes = 2;
			} else if(type == "zombie"){
				numhands = 1;
				numfingers = 4;
				actions_left = 2;
				MAX_ACTIONS = 2;			//ZOMBIE HAS 2 ACTIONS PER TURN
			} else if(type == "doggo"){
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
		void attack(string attacking_part, Player &other_player, string defending_part){
				
				//attacking hand #
				int attacking_number = toupper(attacking_part[1]) - 65;
				
				//defending hand #
				int defending_number = toupper(defending_part[1]) - 65;
				
				//ATTACK WITH HAND
				if(attacking_part[0] == 'H'){
					
					//ATTACK HAND WITH HAND
					if(defending_part[0] == 'H'){
						
						//update fingers of defending hand
						(*other_player.get_hands())[defending_number].add_digits(hands[attacking_number].get_digits());		
						
						//if defending player is a zombie AND 1 lang ang kamay nila tapos nadeads un, regrow
						if(other_player.get_type() == "zombie"){
							if(defending_number == 0 && !(*other_player.get_hands())[defending_number].is_living() && (*other_player.get_hands()).size() == 1){
								other_player.regrow();
							}
						}
					} 
					
					//ATTACK FEET WITH HAND
					else{
						
						//update toes of defending feet
						(*other_player.get_feet())[defending_number].add_digits(hands[attacking_number].get_digits());
						
						//if defending player is non-alien and their defending feet dies, they will earn a skip
						if(!(*other_player.get_feet())[defending_number].is_living() && other_player.get_type() != "alien"){
							other_player.set_skip(true);
						}
					}
				} 
				
				//ATTACK WITH FEET
				else{
					
					//ATTACK HAND WITH FEET
					if(defending_part[0] == 'H'){
						
						//update fingers of defending hand
						(*other_player.get_hands())[defending_number].add_digits(feet[attacking_number].get_digits());
						
						//if defending player is a zombie AND 1 lang ang kamay nila tapos nadeads un, regrow
						if(other_player.get_type() == "zombie"){
							if(defending_number == 0 && !(*other_player.get_hands())[defending_number].is_living() && (*other_player.get_hands()).size() == 1){
								other_player.regrow();
							}
						}
						
					} 
					
					//ATTACK FEET WITH FEET
					else{
						
						//update toes of defending feet
						(*other_player.get_feet())[defending_number].add_digits(feet[attacking_number].get_digits());
						
						//if defending player is non-alien and their defending feet dies, they will earn a skip
						if(!(*other_player.get_feet())[defending_number].is_living() && other_player.get_type() != "alien"){
							other_player.set_skip(true);
						}
					}
				}
				
				//NON-DOGGOS ATTACKING DOGGOS WILL EARN SKIPS; DONT HURT GOOD BOI
				if(type != "doggo" && other_player.get_type() == "doggo"){
					skip = true;
				}
				
				//if this function is called, it means an action has been consumed
				actions_left--;
		}

		//changing the finger values of your hands; disthand
		void transfer_hands(vector<int> hand_values){
			
			for(int i = 0; i < hand_values.size(); i++){
				if(hands[i].is_living()) hands[i].set_digits(hand_values[i]); //only edit hands that are still alive
			}

			actions_left--;
		}
		
		//changing toe values of feet; distfeet
		void transfer_feet(vector<int> feet_values){
			
			for(int i = 0; i < feet_values.size(); i++){
				if(feet[i].is_living()) feet[i].set_digits(feet_values[i]);	//only edit feet that are still alive
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

class Team{
	protected:
		vector<Player> roster;		//list of players in team
		int team_number;			//team number
		int team_size;				//size of team
		bool living;				//status of team
		
	public:
		
		//constructor
		Team(int i){
			team_number = i;
			living = true;
			team_size = 0;
		}
		
		//add player to team
		void add_player(Player new_player){
			roster.push_back(new_player);
			team_size++;
		}
		
		//get player at certain position
		Player get_player(int i){
			return roster[i];
		}
		
		//if atleast one player is living then the team is still alive
		bool is_living(){
			for(int i = 0; i < roster.size(); i++){
				if(roster[i].is_living()){
					return true;
				}
			}
			return false;
		}
		
		//set team number
		void set_team_number(int x){
			team_number = x;
		}
		
		//get team number
		int get_team_number(){
			return team_number;
		}		
	
		//get team size
		int get_team_size(){
			return team_size;
		}
		
		//get list of players
		vector <Player> *get_roster(){
			return &roster;
		}
		
		//check if all players are skipping
		bool skip_team(){
			for(int i = 0 ; i < roster.size(); i++){
				if(roster[i].get_skip() == false && roster[i].is_living() == true){ 	//a living player not set for skipping means the team is not skipping
					return false;
				}
			}
			return true;
		}
		
		//COMPARE TEAM OBJECTS USING THEIR TEAM NUMBER
		bool operator < (const Team& other_team) const
		{
		    return (team_number < other_team.team_number);
		}
};

//COUNT LIVING TEAMS
int count_living_teams(vector<Team> teams){
	int count = 0;
	
	for(int i = 0; i < teams.size(); i++){
		if(teams[i].is_living()){
			count++;
		}
	}
	
	return count;
}


//DISPLAY STATE
void display_state(vector<Team> teams){

	sort(teams.begin(), teams.end());
	//i is team number
	for(int i = 0; i < teams.size();i++){
		cout << "Team " << teams[i].get_team_number() << ": ";
		
		sort(teams[i].get_roster()->begin(),teams[i].get_roster()->end());
		
		//j is player
		for(int j = 0; j < teams[i].get_roster()->size(); j++){
			
			Player current_player = (*teams[i].get_roster())[j];
			
			cout << "P" << current_player.get_player_number() << current_player.get_type()[0];
			cout << " (";

			//k is the hand of each player
			for(int k = 0; k < current_player.get_hands()->size(); k++){
				if((*current_player.get_hands())[k].is_living() == false){
					cout << 'X';
					continue;
				}
				cout << (*current_player.get_hands())[k].get_digits();
			}
			
			cout << ":";
			
			//k is the hand of each player
			for(int k = 0; k < current_player.get_feet()->size(); k++){
				if((*current_player.get_feet())[k].is_living() == false){
					cout << 'X';
					continue;
				}
				cout << (*current_player.get_feet())[k].get_digits();
			}
			
			cout << ")";
			
			if(j < teams[i].get_roster()->size()-1){
				cout << " | ";
			}

		}
		cout << endl;
	}
}

int main(){
	
	int numofplayers, numofteams;
	cin >> numofplayers >> numofteams;
	
	vector<Team> teams;
	
	for(int i = 0; i < numofteams; i++){
		teams.push_back(Team(i+1));
	}

	for(int i = 0; i < numofplayers; i++){
		string temp_type;
		cin >> temp_type;
		int temp_team;
		cin >> temp_team;
		
		teams[temp_team-1].add_player(Player(temp_type,i+1));
	}

	display_state(teams);
	cout << endl;

	//Round number for debugging purposes
	int round_number = 0;
	
	//NEW ROUND WHILE MORE THAN 1 TEAM IS ALIVE
	while(count_living_teams(teams) > 1){
		
		//cout << "TURN NUMBER #"<< round_number << endl;
		
		//TEAM DOESN'T SKIP
		if(!teams[0].skip_team()){
			
			//cout << "Team #" << teams[0].get_team_number() << "'s turn.";
			
			//WHILE NEXT PLAYER DUE IS SKIPPING, GET NEXT PLAYER
			while((*teams[0].get_roster())[0].get_skip() || (*teams[0].get_roster())[0].is_living() == false){
				
				//TURN OFF NEXT PLAYER'S SKIPPING; IT MEANS NA SKIP NA SIYA
				(*teams[0].get_roster())[0].set_skip(false);
				
				//GET NEXT PLAYER
				rotate(teams[0].get_roster()->begin(), teams[0].get_roster()->begin()+1, teams[0].get_roster()->end());
			}
			
			//cout << " Player #" << teams[0].get_roster()->at(0).get_player_number() << " is playing.";
			
			//WHILE ACTIONS LEFT
			while((*teams[0].get_roster())[0].get_actions() > 0){ 
				
				//cout << " Actions left: " << teams[0].get_roster()->at(0).get_actions() << endl << endl;
				
				//cout << "Enter command:" << endl;
				string command;
				cin >> command;
			
				//TAPPING
				if(command == "tap"){
					string apart, tpart;
					int pnumber;
					
					cin >> apart >> pnumber >> tpart;
					
					//LOCATE DEFENDING PLAYER THROUGH PLAYING NUMBER
					Player *other_player = nullptr;
					for(int i = 1; i < teams.size();i++){
						for(int j = 0; j < teams[i].get_roster()->size(); j++){
							//cout << "Checking: Player#" << (*teams[i].get_roster())[j].get_player_number() << endl;											
							if((*teams[i].get_roster())[j].get_player_number() == pnumber){
								
								other_player = &((*teams[i].get_roster())[j]);
							}
						}
					}
					 
					//cout << endl << "Player #" << teams[0].get_roster()->at(0).get_player_number() << " has attacked Player #" << other_player->get_player_number() << endl;
					(*teams[0].get_roster())[0].attack(apart,*other_player,tpart);
				}
				
				//DISTRIBUTING HANDS
				if(command == "disthands"){
					vector<int> new_values;
					
					for(int i = 0; i < teams[0].get_roster()->at(0).get_hands()->size(); i++){
						int x;
						cin >> x;
						new_values.push_back(x);
					}
					
					teams[0].get_roster()->at(0).transfer_hands(new_values);
				}
				
				//DISTRIBUTING FEET
				if(command == "distfeet"){
					vector<int> new_values;
					
					for(int i = 0; i < teams[0].get_roster()->at(0).get_feet()->size(); i++){
						int x;
						cin >> x;
						new_values.push_back(x);
					}
					
					teams[0].get_roster()->at(0).transfer_feet(new_values);
				}
		
				
				//IF WIN CONDITION IS SATISFIED, END ROUND EVEN IF ACTIONS LEFT
				if(count_living_teams(teams) == 1) break;
			}
			
			//DISPLAY STATE AFTER EVERY ROUND NOT ACTION
			//cout << endl << "END-OF-TURN STATS" << endl << "****************************" << endl;
			display_state(teams);	
			//cout << "****************************" <<endl;
			
			
			//REPLENISH PLAYERS ACTIONS LEFT
			teams[0].get_roster()->at(0).reset_actions();
			
			//GET NEXT PLAYER
			rotate(teams[0].get_roster()->begin(), teams[0].get_roster()->begin()+1, teams[0].get_roster()->end());
			//cout << "(players have been rotated)" << endl;
			
		} 
		//TEAM IS SKIPPING
		else{
			//cout << "> Team #" << teams[0].get_team_number() << " is skipping." <<endl;
			
			//RESET THE ENTIRE TEAM'S SKIPS
			for(int j = 0; j < teams[0].get_roster()->size(); j++){								
				(*teams[0].get_roster())[j].set_skip(false);		
			}
			
		}

		//GET NEXT TEAM
		rotate(teams.begin(), teams.begin()+1, teams.end());
		//cout << "(teams have been rotated)" << endl;
		//cout << "E N D  O F  T U R N" << endl;
		//cout <<"---------------------------------------------------------"<<endl<<endl;
		
		round_number++;
	}
	
	//FIND THE TEAM NUMBER OF THE LAST LIVING TEAM AND DISPLAY WINNER
	for(int i = 0; i < teams.size(); i++){
		if(teams[i].is_living()){
			cout<<"Team "<<teams[i].get_team_number()<<" wins!"<<endl;
		}
	}
}
