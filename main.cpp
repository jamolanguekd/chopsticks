#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#include "socketstream/socketstream.hh"
#include "Hand.h"
#include "Foot.h"
#include "Player.h"
#include "Team.h"
#include "misc.h"

using namespace std;
using namespace swoope;

void parse_command(vector<string> & command, string s){
	stringstream ss(s);

	while(ss){
		string temp;
		ss >> temp;
		command.push_back(temp);
	}
}

void broadcast(string s, socketstream ss[], int size){
	cout << s << endl;

	for(int i = 1; i < size; i++){
		ss[i] << s+'*' << endl;
	}
}

void receive(socketstream &s){
	string temp;
	getline(s, temp,'*');
	cout << temp << endl;
}

int runServer(int port){
	
	int PLAYER_SIZE;

	cout << "How many people are playing? (2-6 Players)" << endl;
	do{
		cin >> PLAYER_SIZE;
		cin.ignore();
		
		if(cin.good() == 0){
			cin.clear();
			cin.ignore();
			cout << "Invalid input. Please try again!" << endl;
			continue;
		}

		if(!(2 <= PLAYER_SIZE and PLAYER_SIZE <= 6)){
			cout << "There can only be 2-6 players in a game." << endl;
		} else{
			break;
		}
	}while(true);

	cout << endl << "Created a game with " << PLAYER_SIZE << " players." << endl;

	socketstream listener;
	listener.open(to_string(port), PLAYER_SIZE);

	//WAITING FOR CONNECTIONS
	socketstream player_sockets[PLAYER_SIZE];
	for(int i = 1; i < PLAYER_SIZE; i++){
		cout << "Waiting for Player #" << i + 1 << "..." << endl;
		listener.accept(player_sockets[i]);
		cout << "Player #" << i + 1 << " has connected!" << endl;
		
		//SEND PLAYER # 
		player_sockets[i] << i+1 << endl;
	}

	cout << endl;
	broadcast("ALL PLAYERS HAVE CONNECTED!", player_sockets, PLAYER_SIZE);
	cout << endl;

	//PLAYER TYPE PREFERENCES
	
	string player_types[PLAYER_SIZE];
	cout << "Please enter your prefered player type: (Human, Alien, Zombie, Doggo)" << endl;
	while(true){

		cin >> player_types[0];
		cin.ignore();

		strupper(player_types[0]);

		if(player_types[0] == "HUMAN" or player_types[0] == "ALIEN" or player_types[0] == "ZOMBIE" or player_types[0] == "DOGGO") break;

		cout << "INVALID PLAYER TYPE! Please select one of the following: (Human, Alien, Zombie, Doggo)" << endl;
		};

	for(int i = 1; i < PLAYER_SIZE; i++){
		player_sockets[i] >> player_types[i];
		player_sockets[i].ignore();
	}

	cout << endl;
	broadcast("PLAYER TYPE PREFERENCES HAVE BEEN SET!", player_sockets, PLAYER_SIZE);
	cout<<endl;

	//TEAM GROUPING PREFERENCES
	
	int player_teams[PLAYER_SIZE];
	broadcast( "Please enter your preferred team number: (Pick a number between 1 - " + to_string(PLAYER_SIZE) + ")", player_sockets, PLAYER_SIZE);
	
	while(true){

		cin >> player_teams[0];
		cin.ignore();

		for(int i = 1; i < PLAYER_SIZE; i++){
			player_sockets[i] >> player_teams[i];
			player_sockets[i].ignore();
		}
		
		bool teams_valid = validate_teams(player_teams, PLAYER_SIZE);
		for(int i = 1; i < PLAYER_SIZE; i++){
			player_sockets[i] << teams_valid << endl;
		}
		
		if(teams_valid) break;

		cout << "INVALID TEAM NUMBER. Please try again: " << endl;
	}

	string temp = "\n";
	for(int i = 0; i < PLAYER_SIZE; i++){
		temp += "Player #" + to_string(i+1) + " has chosen Team #" + to_string(player_teams[i]) + ".\n";
	}
	broadcast(temp, player_sockets, PLAYER_SIZE);
	cout << endl;
	broadcast("TEAMS HAVE BEEN SET!", player_sockets, PLAYER_SIZE);
	cout << endl;

	//ACTUAL GAME START
	int TEAM_SIZE = max_val(player_teams, PLAYER_SIZE);

	vector<Team> teams;
	for(int i = 0; i < TEAM_SIZE; i++){
		teams.push_back(Team(i+1));
	}

	for(int i = 0; i < PLAYER_SIZE; i++){
		string stype = player_types[i];
		teams[player_teams[i]-1].add_player(Player(stype,i+1,player_teams[i],&player_sockets[i]));
	}

	cout << endl;
	broadcast("---------------------GAME START----------------------", player_sockets, PLAYER_SIZE);
	cout << endl;
	broadcast(display_state(teams), player_sockets, PLAYER_SIZE);
	cout << endl;
}

void runClient(int port, string ip){
	
	//CONNECT TO SERVER
	socketstream server;
	server.open(ip, to_string(port));

	//RECEIVE PLAYER #
	int player_number;
	server >> player_number;
	server.ignore();

	cout << "You have joined a game! You are Player #" << player_number << "." << endl;
	cout << "Waiting for the rest of the players..." << endl;
	
	cout << endl;
	receive(server);
	cout << endl;

	//PLAYER TYPE PREFERENCES
	
	cout << "Please enter your preferred player type: (Human, Alien, Zombie, Doggo)" << endl;

	string type;
	while(true){

		cin >> type;
		cin.ignore();
		
		strupper(type);

		if(type == "HUMAN" or type == "ALIEN" or type == "ZOMBIE" or type == "DOGGO") break;
	
		cout << "INVALID PLAYER TYPE! Please select one of the following: (Human, Alien, Zombie, Doggo)" << endl;
	};

	server << strupper(type) << endl;

	cout << endl;
	receive(server);
	cout << endl;

	//TEAM GROUPING PREFERENCES
	
	receive(server);

	while(true){

		int team_number;
		cin >> team_number;			
		cin.ignore();
		server << team_number << endl;

		bool b = false;

		server >> b;
		server.ignore();
		
		if(b) break;

		cout << "INVALID TEAM NUMBER! Please try again: " << endl;
	}
	receive(server);
	cout << endl;
	receive(server);
	cout << endl;

	//ACTUAL GAME START
	
	receive(server);
	cout << endl;
	receive(server);
	cout << endl;
}

int setup(int argc, char* argv[]){
	if(argc < 1 and argc > 2){
		cout << "Too many arguments. The program will not exit." << endl;
		return 0;
	}

	int port = atoi(argv[1]);
	string ip = (argv[2] == NULL? "" :  argv[2]);

	if(1024 <= port and port <= 65535){
		if(ip == "") runServer(port);
		else runClient(port, ip);
	}
	else{
		cout << "Invalid port number!The port number must be between 1024-65535." << endl;
		cout << "The program will now exit..." << endl;
		return 0;
	}
}

int main(int argc, char *argv[]){

	setup(argc,argv);	
	/*
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
		teams[temp_team-1].add_player(Player(strupper(temp_type),i+1,temp_team-1));
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
				if(strupper(command[0]) == "TAP"){
					string apart = strupper(command[1]);
					string tpart = strupper(command[3]);
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
				else if(strupper(command[0]) == "DISTHANDS"){
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
				else if(strupper(command[0]) == "DISTFEET"){
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
	*/
}
