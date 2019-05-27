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
	while(true){
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
	}

	cout << "Created a game with " << PLAYER_SIZE << " players." << endl;

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

	broadcast("ALL PLAYERS HAVE CONNECTED!", player_sockets, PLAYER_SIZE);

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

	broadcast("PLAYER TYPE PREFERENCES HAVE BEEN SET!", player_sockets, PLAYER_SIZE);

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
		temp += "Player #" + to_string(i+1) + " has chosen Team #" + to_string(player_teams[i]) + ".";
	}
	broadcast(temp, player_sockets, PLAYER_SIZE);
	broadcast("TEAMS HAVE BEEN SET!", player_sockets, PLAYER_SIZE);

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

	broadcast("---------------------GAME START----------------------", player_sockets, PLAYER_SIZE);
	broadcast(display_state(teams), player_sockets, PLAYER_SIZE);

	int turn_number = 1;

	bool gameOver = false;
	while(!gameOver){
		
		//BROADCAST TURN NUMBER
		broadcast("######## Turn #" + to_string(turn_number)+" ########", player_sockets, PLAYER_SIZE);

		Team *current_team = &(teams[0]);
		vector<Player> *current_roster = current_team->get_roster();
		int current_team_number = current_team->get_team_number();

	       	
		//SEND SKIP TEAM CHECK RESULTS
		bool teamSkip = current_team->skip_team();
		for(int i = 1; i < PLAYER_SIZE; i++){
			player_sockets[i] << teamSkip << endl;
		}
		if(!teamSkip){

			Player *current_player = &(current_roster->at(0));
			int current_player_number = current_player->get_player_number();

			//BROADCAST TEAM NUMBER
			broadcast("Team #" + to_string(current_team_number) + " is playing.", player_sockets, PLAYER_SIZE);

			//SEND NEXT PLAYER STATUS
			bool nextPlayerFound = (current_player->get_skip() == false and current_player->is_living() == true ? true: false);
			for(int i = 1; i < PLAYER_SIZE; i++){
				player_sockets[i] << nextPlayerFound << endl;
			}
			while(!nextPlayerFound)
			{
				//BROADCAST PLAYER SKIP
				broadcast("Player #" + to_string(current_player_number) + "is skipping. Getting next player...", player_sockets, PLAYER_SIZE);
				current_player->set_skip(false);
				
				//ROTATE
				rotate(current_roster->begin(), current_roster->begin()+1, current_roster->end());
				current_player = &(current_roster->at(0));
				current_player_number = current_player->get_player_number();

				//RESEND NEXT PLAYER STATUS
				nextPlayerFound = (current_player->get_skip() == false and current_player->is_living() == true ? true: false);
				for(int i = 0; i < PLAYER_SIZE; i++){
					player_sockets[i] << nextPlayerFound << endl;
				}
			}

			//BROADCAST PLAYER TURN
			int playingNumber = current_player_number;
			broadcast("Player #" + to_string(playingNumber) + " is playing.", player_sockets, PLAYER_SIZE);

			//SEND PLAYER # OF PLAYING PLAYER
			for(int i = 1; i < PLAYER_SIZE; i++){
				player_sockets[i] << playingNumber << endl;
			}

			//SEND ACTIONS LEFT
			int actions_left = current_player->get_actions();
			for(int i = 1; i < PLAYER_SIZE; i++){
				player_sockets[i] << actions_left << endl;
			}
			while(actions_left > 0){
				
				//BROADCAST ACTIONS LEFT
				broadcast("Actions left: " + to_string(actions_left), player_sockets, PLAYER_SIZE);
				cout << endl;

				vector<string> parsedCommand;
				string unparsedCommand;

				//GET COMMAND
				if(playingNumber == 1){
					cout << "What do you want to do? (TAP | DISTHANDS | DISTFEET)" << endl;
					getline(cin, unparsedCommand);
				}
				else{
					cout << "Waiting for input from Player #" << playingNumber << "..." << endl;
					cout << endl;
					getline(player_sockets[playingNumber-1], unparsedCommand); 
				}

				parse_command(parsedCommand, unparsedCommand);
			
				string commandStatus = "";

				//ATTACK
				if(strupper(parsedCommand[0]) == "TAP"){
					string apart = strupper(parsedCommand[1]);
					string tpart = strupper(parsedCommand[3]);
					int pnumber = stoi(parsedCommand[2]);

					//LOCATE DEFENDING PLAYER
					Player *other_player = nullptr;

					for(int i = 0; i < teams.size(); i++){
						for(int j = 0; j < teams[i].get_roster()->size(); j++){
							if(teams[i].get_roster()->at(j).get_player_number() == pnumber){
								other_player = &(teams[i].get_roster()->at(j));
							}
						}
					}

					if(other_player == nullptr){
						commandStatus = "INVALID MOVE! Player #" + to_string(pnumber) + " does not exist. Please try again.";
					} else if(other_player->is_living() == false){
						commandStatus = "INVALID MOVE! Player #" + to_string(pnumber) + " is already dead. Please try again.";
					} else if(current_team_number  == other_player->get_player_team_number()){
							if(current_player_number == other_player->get_player_number()){
									commandStatus = "INVALID MOVE! You cannot attack yourself. Please try again.";
							}else{
									commandStatus = "INVALID MOVE! You cannot attack your teammates. Please try again.";
							}
					} else{
						commandStatus = current_player->attack(apart,*other_player,tpart);
					}
				}

				//DISTHANDS
				else if(strupper(parsedCommand[0]) == "DISTHANDS"){
					
					if(current_player->count_living_hands() == 1){
						commandStatus = "INVALID MOVE! You only have one hand left.";
					}

					else{

						vector<int> new_values;

						for(int i = 0; i < current_player->count_living_hands(); i++){
							int x;
							x = stoi(parsedCommand[i+1]);
							new_values.push_back(x);
						}

						if(current_player->validate_transfer_hands(new_values)){
							current_player->transfer_hands(new_values);
							commandStatus = "Player #" + to_string(current_player_number) +  " has redistributed hands."; 	
						} else{
							commandStatus = "INVALID MOVE! Redistribution is not valid.";
						}
					}
				}

				//DISTFEET
				else if(strupper(parsedCommand[0]) == "DISTFEET"){
					
					if(current_player->count_living_feet() == 1){
						commandStatus = "INVALID MOVE! You only have one hand left.";
						break;
					}
						
					else{
						vector<int> new_values;

						for(int i = 0; i < current_player->count_living_feet(); i++){
							int x;
							x = stoi(parsedCommand[i+1]);
							new_values.push_back(x);
						}

						if(current_player->validate_transfer_feet(new_values)){
							current_player->transfer_feet(new_values);
							commandStatus = "Player #" + to_string(current_player_number) + " has redistributed feet.";
						} else{
							commandStatus  = "INVALID MOVE! Redistribution is not valid.";
						}
					}
				}

				//UNRECOGNIZED MOVE
				else{
					commandStatus = "INVALID MOVE! That command does not exist. Please try again.";
				}

				//BROADCAST COMMAND STATUS
				/*if(playingNumber == 1){
					cout << commandStatus << endl;
					for(int i = 0; i < PLAYER_SIZE; i++){
						player_sockets[i] << "" << endl;
					}
				} else{
					for(int i = 1; i < PLAYER_SIZE; i++){
						if(i == playingNumber-1){
							player_sockets[i] << commandStatus << endl;
						} else{
							string s ="";
							player_sockets[i] << s << endl;
						}
					}
				}*/ broadcast(commandStatus, player_sockets, PLAYER_SIZE);
							 
				//RESEND ACTIONS LEFT
				actions_left = current_player->get_actions();
				for(int i = 0; i < PLAYER_SIZE; i++){
					player_sockets[i] << actions_left << endl;
				}

				//DISPLAY STATE
				broadcast(display_state(teams), player_sockets, PLAYER_SIZE);

				if(count_living_teams(teams) == 1) break;

			}

			//REPLENISH
			current_player->reset_actions();

			//ROTATE NEXT PLAYER
			rotate(current_roster->begin(), current_roster->begin()+1, current_roster->end());	
		}

		else{

			//BROADCAST THE SKIPPING TEAM
			broadcast("Team #" + to_string(current_team_number) + " is skipping.", player_sockets, PLAYER_SIZE);

			//RESET THE SKIPS
			for(int j = 0; j < current_roster->size(); j++){
				current_roster->at(j).set_skip(false);
			}

		}

		//ROTATE TEAMS
		rotate(teams.begin(), teams.begin()+1, teams.end());


		//SEND GAME OVER STATUS
		gameOver = (count_living_teams(teams) <= 1 ? true : false);
		for(int i = 1; i < PLAYER_SIZE; i++){
			player_sockets[i] << gameOver << endl;
		}

		turn_number++;
	}


	int winning_team;

	//BROADCAST WINNING TEAM
	for(int i = 0; i < teams.size(); i++){
		if(teams[i].is_living()){
			string winning = "Team #" + to_string(teams[i].get_team_number()) + " is the winner!";
			broadcast(winning, player_sockets, PLAYER_SIZE);

			winning_team = teams[i].get_team_number();
		}
	}

	//BROADCAST INDIVIDUAL STATUS
	if(player_teams[0] == winning_team){
		cout << "YOU WIN!" << endl;
	} else{
		cout << "YOU LOSE!" << endl;
	}

	for(int i = 1; i < PLAYER_SIZE; i++){
		if(player_teams[i] == winning_team){
			player_sockets[i] << "YOU WIN!" << endl;
		} else{
			player_sockets[i] << "YOU LOSE!" << endl;
		}
	}




	return 0;
}

int runClient(int port, string ip){
	
	//CONNECT TO SERVER
	socketstream server;
	server.open(ip, to_string(port));

	//RECEIVE PLAYER #
	int player_number;
	server >> player_number;
	server.ignore();

	cout << "You have joined a game! You are Player #" << player_number << "." << endl;
	cout << "Waiting for the rest of the players..." << endl;
	
	receive(server);

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

	receive(server);

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
	receive(server);

	//ACTUAL GAME START 

	receive(server);
	receive(server);

	bool gameOver = false;
	while(!gameOver){

		//RECEIVER TURN NUMBER
		receive(server);

		//RECEIVE TEAM SKIP CHECK RESULTS
		bool teamSkip;
		server >> teamSkip;
		server.ignore();

		if(!teamSkip){

			//RECEIVE TEAM NUMBER
			receive(server);

			//RECEIVE NEXT PLAYER STATUS
			bool nextPlayerFound;
			server >> nextPlayerFound;
			server.ignore();

			while(!nextPlayerFound){
				//RECEIVE PLAYER SKIP
				receive(server);
				//SERVER WILL ROTATE
				//RECEIVE NEXT PLAYER STATUS
				server >> nextPlayerFound;
				server.ignore();
			}

			//RECEIVE PLAYER TURN
			receive(server);

			int playingNumber;
			server >> playingNumber;
			server.ignore();

			//RECEIVE ACTIONS LEFT
			int actions_left;
			server >> actions_left;
			server.ignore();

			while(actions_left > 0){
				
				//RECEIVE ACTIONS LEFT
				receive(server);
			
				string unparsedCommand;

				if(playingNumber == player_number){

					cout << "What do you want to do? (TAP | DISTHANDS | DISTFEET)" << endl;
					getline(cin, unparsedCommand);
					server << unparsedCommand << endl;
				}
				else{
					cout << "Waiting for input from Player #" << playingNumber << "..." <<  endl;
				}

				
				//RECEIVE COMMAND STATUS
				receive(server);

				//REFRESH ACTIONS LEFT
				server >> actions_left;
				server.ignore();

				//RECEIVE DISPLAY STATE
				receive(server);
			}
	
		} else{
			//RECEIVE SKIPPING TEAM
			receive(server);
		}

		//RECEIVE GAME OVER STATUS
		server >> gameOver;
		server.ignore();
	}

	//RECEIVE WINNING 
	receive(server);

	//RECEIVE INDIVIDUAL STATUS
	receive(server);
	return 0;

}

int setup(int argc, char* argv[]){
	if(argc < 1 and argc > 2){
		cout << "Too many arguments. The program will not exit." << endl;
		return 0;
	}

	int port = atoi(argv[1]);
	string ip = (argv[2] == NULL? "" :  argv[2]);

	if(1024 <= port and port <= 65535){
		if(ip == "") return runServer(port);
		else return runClient(port, ip);
	}
	else{
		cout << "Invalid port number!The port number must be between 1024-65535." << endl;
		cout << "The program will now exit..." << endl;
		return 0;
	}
}

int main(int argc, char *argv[]){

	return setup(argc,argv);	
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
