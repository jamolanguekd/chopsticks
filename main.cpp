#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#include "Hand.h"
#include "Foot.h"
#include "Player.h"
#include "Team.h"
#include "misc.h"

using namespace std;
void  parse_command(vector<string> & command, string s){
	stringstream ss(s);

	while(ss){
		string temp;
		ss >> temp;
		command.push_back(temp);
	}
}

int setup(int argc, char* argv[]){
	if(argc < 1 and argc > 2){
		cout << "Too many arguments. The program will not exit." << endl;
		return 0;
	}

	int port = atoi(argv[1]);
	string ip = (argv[2] == NULL? "" :  argv[2]);

	if(1024 <= port and port <= 65535){
		//run server
	}
	else{
		//run client
	}
}

int runServer(int port){

}

void runClient(int){


}

int main(int argc, char *argv[]){

	setup(argc,argv);	

	int numofplayers, numofteams;
	cin >> numofplayers;
	cin.ignore();

	cin >> numofteams;
	cin.ignore();

	vector<Team> teams;
	
	for(int i = 0; i < numofteams; i++){
		teams.push_back(Team(i+1));
	}

	for(int i = 0; i < numofplayers; i++){
		string temp_type;
		cin >> temp_type;
		cin.ignore();
		int temp_team;
		cin >> temp_team;
		cin.ignore();
		teams[temp_team-1].add_player(Player(temp_type,i+1,temp_team-1));
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
				vector<string> command;
				
				string temp;
				getline(cin, temp);

				parse_command(command, temp);


				//TAPPING
				if(command[0] == "tap"){
					string apart = command[1];
					string tpart = command[3];
					int pnumber = stoi(command[2]);
					
					//LOCATE DEFENDING PLAYER THROUGH PLAYING NUMBER
					Player *other_player = nullptr;
					for(int i = 0; i < teams.size();i++){
						for(int j = 0; j < teams[i].get_roster()->size(); j++){
							//cout << "Checking: Player#" << (*teams[i].get_roster())[j].get_player_number() << endl;											
							if((*teams[i].get_roster())[j].get_player_number() == pnumber){
								
								other_player = &((*teams[i].get_roster())[j]);
							}
						}
					}

					if(other_player == nullptr){
						cout << "INVALID MOVE! Player #" << pnumber << " does not exist." << endl;
					}

					else{
						if(other_player->get_player_team_number()  == teams[0].get_roster()->at(0).get_player_team_number()){
							if(pnumber == teams[0].get_roster()->at(0).get_player_number()){
								cout << "INVALID MOVE! You cannot attack yourself." << endl;
							}
							else{
								cout << "INVALID MOVE! You cannot attack your teammates " << endl;
						
							}
						}

						else{
							//cout << endl << "Player #" << teams[0].get_roster()->at(0).get_player_number() << " has attacked Player #" << other_player->get_player_number() << endl;
							(*teams[0].get_roster())[0].attack(apart,*other_player,tpart);
						}
					}
				}
				
				//DISTRIBUTING HANDS
				else if(command[0] == "disthands"){
					vector<int> new_values;
					
					for(int i = 0; i < teams[0].get_roster()->at(0).get_hands()->size(); i++){
						new_values.push_back(stoi(command[1+i]));
					}
					
					if(teams[0].get_roster()->at(0).validate_transfer_hands(new_values)){
						teams[0].get_roster()->at(0).transfer_hands(new_values);
					}

					else{
						cout << "INVALID MOVE! Your new finger count does not match your old count." << endl;
					}
				}
				
				//DISTRIBUTING FEET
				else if(command[0] == "distfeet"){
					vector<int> new_values;
					
					for(int i = 0; i < teams[0].get_roster()->at(0).get_feet()->size(); i++){
						new_values.push_back(stoi(command[1+i]));
					}
					if(teams[0].get_roster()->at(0).validate_transfer_feet(new_values)){
						teams[0].get_roster()->at(0).transfer_feet(new_values);
					}
					else{
						cout << "INVALID MOVE! Your new toe count does not match your old toe count." << endl;
					}
				}
		
				else{
					cout << "INVALID MOVE! The command does not exist." << endl;
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
