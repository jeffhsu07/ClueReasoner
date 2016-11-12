#include "ClueReasoner.h"
using namespace std;

int ClueReasoner::GetPlayerNum(string player)
{
	if (player == case_file)
		return num_players;
	
	for (int i = 0; i < num_players; i++)
		if (player == players[i])
			return i;
			
	cout<<"Illegal player: "<<player<<endl;
	return -1;
}
int ClueReasoner::GetCardNum(string card)
{
	for (int i = 0; i < num_cards; i++)
		if (card == cards[i])
			return i;
			
	cout<<"Illegal card: "<<card<<endl;
	return -1;
}

string ClueReasoner::QueryString(int return_code)
{
	if (return_code == kFalse)
		return "n";
	else if (return_code == kTrue)
		return "Y";
	else
		return "-";
}

void ClueReasoner::PrintNotepad()
{
	for (int i = 0; i < num_players; i++)
		cout<<"\t"<<players[i];
	cout<<"\t"<<case_file<<endl;
	
	for (int i = 0; i < num_cards; i++)
	{
		cout<<cards[i]<<"\t";
		for (int j = 0; j < num_players; j++)
			cout<<QueryString(Query(players[j], cards[i]))<<"\t";
		
		cout<<QueryString(Query(case_file, cards[i]))<<endl;
	}
}
	
void ClueReasoner::AddInitialClauses()
{
	/* The following code is given as an example to show how to create Clauses and post them to the solver. SatSolver.h uses the following typedefs:
		typedef int Literal;
		typedef std::vector<Literal> Clause;
		
	That is, a Literal (a propositional variable or its negation) is defined as a positive or a negative (meaning that it is in negated form, as in -p or -q) integer, and a Clause is defined as a vector of Literals.
	
	The function GetPairNum(p, c) returns the literal that corresponds to card c being at location p (either a player's hand or the case_file). 
	See ClueReasoner.h, lines 7-29 for a definition of the arrays and variables that you can use in your implementation. 
	*/

	// Each card is in at least one place (including the case file).
	for (int c = 0; c < num_cards; c++)	// Iterate over all cards.
	{
		Clause clause;
		for (int p = 0; p <= num_players; p++)	// Iterate over all players, including the case file (as a possible place for a card).
			clause.push_back(GetPairNum(p, c));
		
		solver->AddClause(clause);
	}
        
	// If a card is in one place, it cannot be in another place.
    for (int c = 0; c < num_cards; c++) // Iterate over all cards.
    {
        for (int p1 = 0; p1 <= num_players; p1++)   // Iterate over all pairs of players.
        {
            for (int p2 = p1 + 1; p2 <= num_players; p2++)
            {
                if (p1 != p2)   // Exclude pairs of same player.
                {
                    Clause clause; // If c is at p1, then c is not at p2.
                    clause.push_back(-1 * GetPairNum(p1, c));
                    clause.push_back(-1 * GetPairNum(p2, c));
                    solver->AddClause(clause);
                }
            }
        }
    }
	
	// At least one card of each category is in the case file.
    Clause clause1;     // At least one suspect.
    for (int s = 0; s < num_suspects; s++)
    {
        clause1.push_back(GetPairNum("cf", suspects[s]));
    }
    solver->AddClause(clause1);
    Clause clause2;     // At least one weapon.
    for (int w = 0; w < num_weapons; w++)
    {
        clause2.push_back(GetPairNum("cf", weapons[w]));
    }
    solver->AddClause(clause2);
    Clause clause3;     // At least one room.
    for (int r = 0; r < num_rooms; r++)
    {
        clause3.push_back(GetPairNum("cf", rooms[r]));
    }
    solver->AddClause(clause3);

	// No two cards in each category can both be in the case file.
    for (int s1 = 0; s1 < num_suspects; s1++)   // No two suspects.
    {
        for (int s2 = s1 + 1; s2 < num_suspects; s2++)
        {
            if (s1 != s2)
            {
                Clause clause;
                clause.push_back(-1 * GetPairNum("cf", suspects[s1]));
                clause.push_back(-1 * GetPairNum("cf", suspects[s2]));
                solver->AddClause(clause);
            }
        }
    }
    for (int w1 = 0; w1 < num_suspects; w1++)   // No two weapons.
    {
        for (int w2 = w1 + 1; w2 < num_suspects; w2++)
        {
            if (w1 != w2)
            {
                Clause clause;
                clause.push_back(-1 * GetPairNum("cf", weapons[w1]));
                clause.push_back(-1 * GetPairNum("cf", weapons[w2]));
                solver->AddClause(clause);
            }
        }
    }
    for (int r1 = 0; r1 < num_suspects; r1++)   // No two rooms.
    {
        for (int r2 = r1 + 1; r2 < num_suspects; r2++)
        {
            if (r1 != r2)
            {
                Clause clause;
                clause.push_back(-1 * GetPairNum("cf", rooms[r1]));
                clause.push_back(-1 * GetPairNum("cf", rooms[r2]));
                solver->AddClause(clause);
            }
        }
    }
	
}
void ClueReasoner::Hand(string player, string cards[3])
{
	// GetPlayerNum returns the index of the player in the players array (not the suspects array). Remember that the players array is sorted wrt the order that the players play. Also note that, player_num (not to be confused with num_players) is a private variable of the ClueReasoner class that is initialized when this function is called.
	player_num = GetPlayerNum(player);
	
    
    for (int i = 0; i < 3; i++)
    {
        Clause clause;
        clause.push_back(GetPairNum(player, cards[i]));
        solver->AddClause(clause);
    }
}
void ClueReasoner::Suggest(string suggester, string card1, string card2, string card3, string refuter, string card_shown)
{
	// Note that in the Java implementation, the refuter and the card_shown can be NULL. 
	// In this C++ implementation, NULL is translated to be the empty string "".
	// To check if refuter is NULL or card_shown is NULL, you should use if(refuter == "") or if(card_shown == ""), respectively.
    
    int suggester_num = GetPlayerNum(suggester);
    
    if (refuter == "")
    {
        // Refuter is null --> All players but the suggester don't have the cards.
        for (int i = 0; i < num_players; i++)
        {
            if (i != suggester_num)
            {
                Clause clause1;
                clause1.push_back(-1 * GetPairNum(players[i], card1));
                solver->AddClause(clause1);
                
                Clause clause2;
                clause2.push_back(-1 * GetPairNum(players[i], card2));
                solver->AddClause(clause2);
                
                Clause clause3;
                clause3.push_back(-1 * GetPairNum(players[i], card3));
                solver->AddClause(clause3);
            }
        }
    }
    else
    {
        int refuter_num = GetPlayerNum(refuter);
        
        // All players between suggester and refuter do not have any of these cards.
        int i = suggester_num;
        if (suggester_num < num_players - 1) i++;
        else i = 0;
        
        while (i != refuter_num)
        {
            Clause clause1;
            clause1.push_back(-1 * GetPairNum(players[i], card1));
            solver->AddClause(clause1);
            
            Clause clause2;
            clause2.push_back(-1 * GetPairNum(players[i], card2));
            solver->AddClause(clause2);
            
            Clause clause3;
            clause3.push_back(-1 * GetPairNum(players[i], card3));
            solver->AddClause(clause3);
            
            // Increment player index
            if (i < num_players - 1) i++;
            else i = 0;
        }
        
        // Card shown is not null --> The refuter has the card shown.
        if (card_shown != "")
        {
            Clause clause;
            clause.push_back(GetPairNum(refuter, card_shown));
            solver->AddClause(clause);
        }
        
        // Card shown is null --> The refuter has at least one of the three cards.
        else
        {
            Clause clause;
            clause.push_back(GetPairNum(refuter, card1));
            clause.push_back(GetPairNum(refuter, card2));
            clause.push_back(GetPairNum(refuter, card3));
            solver->AddClause(clause);
        }
    }
    
}
void ClueReasoner::Accuse(string suggester, string card1, string card2, string card3, bool is_correct)
{
	// TO BE IMPLEMENTED AS AN EXERCISE
}
