#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "socketstream/socketstream.hh"
#include "Player.h"

using namespace std;
using namespace swoope;

#ifndef TEAM_H
#define TEAM_H
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
#endif
