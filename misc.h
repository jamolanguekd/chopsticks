#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

//get max value from array
int max_val(int arr[], int size){
	int x = -1;
	for(int i = 0; i < size; i++){
		x = max(x, arr[i]);
	}

	return x;
}

//validate grouping preferences
bool validate_teams(int arr[], int size){

	int tcsize = max_val(arr,size);

	int team_count[tcsize];
	fill(team_count, team_count+tcsize, 0);
	for(int i = 0; i < size; i++){
		team_count[arr[i]-1] += 1;
	}

	int count = 0; //count how many teams have more than one player
	for(int i = 0; i < tcsize; i++){
		count = count + (team_count[i] > 0 ? 1 : 0);
	}

	bool no_gaps = true;
	for(int i = 0; i < tcsize-1; i++){
		if(team_count[i] == 0 && team_count[i+1] != 0){
			no_gaps = false;
			break;
		}
	}

	return count > 1 && no_gaps;
}

//capitalizing strings
string strupper(string &s){
	for(int i = 0; i < s.size(); i++){
		s[i] = toupper(s[i]);
	}

	return s;
}

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
			
			cout << "P" << current_player.get_player_number() <<  char(tolower(current_player.get_type()[0]));
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

