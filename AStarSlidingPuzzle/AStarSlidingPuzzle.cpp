/*
Mychael B Hunt
mychaelhunt@gmail.com
A* vs Breadth First Search Algorithms for Solving the Sliding Puzzle

Sources:
https://towardsdatascience.com/solve-slide-puzzle-with-hill-climbing-search-algorithm-d7fb93321325
https://picoledelimao.github.io/blog/2015/12/06/solving-the-sliding-puzzle/
*/


#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <queue>
// #include <deque>
using namespace std;

int dimension = 0;
int** board = 0;
vector<int> finalPath;
int mainlastMove = 0;

enum Direction { LEFT, RIGHT, UP, DOWN, INVALID };

struct Instance {
	int** board = 0;
	vector<int> path;
	int* lastMove = new int(-1);
};

void printBoard(int** thisBoard);
void deleteInstance(Instance thisInstance);

void setupPuzzle(int dim) {
	dimension = dim;
	board = new int*[dimension]; // allocate space

								 // fill the board
	for (int i = 0; i < dimension; i++)
	{
		board[i] = new int[dimension];

		for (int j = 0; j < dimension; j++)
		{
			if (i == dimension - 1 && j == dimension - 1) { // if at last value
				board[i][j] = 0;
			}
			else {
				board[i][j] = (dimension * i + j + 1);
				// dimension * i so that the other rows start with the length of the dimension. (so in a 3x3, the second row will add 3, then add 0 for the j value, then +1 so the first value of the second row will be 4)
			}
		}
	}
}

// Get the (x, y) position of the blank space
int* getBlankSpacePosition(int** thisBoard) {
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			if (thisBoard[i][j] == 0) {
				int* newInt = new int[2];
				newInt[0] = i;
				newInt[1] = j;
				return newInt;
			}
		}
	}
	return nullptr;
}

// Swap two items on a bidemensional array
void swap(int i1, int j1, int i2, int j2, int** thisBoard) {
	int temp = thisBoard[i1][j1];
	thisBoard[i1][j1] = thisBoard[i2][j2];
	thisBoard[i2][j2] = temp;
}

// Return the direction that a piece can be moved, if any
Direction getMove(int piece, int** thisBoard) {
	int* blankPosition = getBlankSpacePosition(thisBoard);
	int line = blankPosition[0];
	int column = blankPosition[1];

	// clean up memory
	delete blankPosition;

	if (line > 0 && piece == thisBoard[line - 1][column]) {
		return DOWN;
	}
	else if (line < dimension - 1 && piece == thisBoard[line + 1][column]) {
		return UP;
	}
	else if (column > 0 && piece == thisBoard[line][column - 1]) {
		return RIGHT;
	}
	else if (column < dimension - 1 && piece == thisBoard[line][column + 1]) {
		return LEFT;
	}
	return INVALID;
}

// Move a piece if possible, and return the direction it was moved
Direction move(int piece, int** thisBoard, int* thisLastMove) {
	Direction move = getMove(piece, thisBoard);
	if (move != INVALID) {
		int* blankPosition = getBlankSpacePosition(thisBoard);
		int line = blankPosition[0];
		int column = blankPosition[1];
		switch (move)
		{
		case LEFT:
			swap(line, column, line, column + 1, thisBoard);
			break;
		case RIGHT:
			swap(line, column, line, column - 1, thisBoard);
			break;
		case UP:
			swap(line, column, line + 1, column, thisBoard);
			break;
		case DOWN:
			swap(line, column, line - 1, column, thisBoard);
			break;
		}
		if (move != INVALID) {
			*thisLastMove = piece;
		}
		return move;
	}
}

bool isGoalState(int** thisBoard) {
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			int piece = thisBoard[i][j];
			if (piece != 0) {
				int originalLine = (int)floor((piece - 1) / dimension);
				int originalColumn = (int)((piece - 1) % dimension);
				if (i != originalLine || j != originalColumn) return false;
			}
		}
	}
	return true;
}

vector<int> getAllowedMoves(int** thisBoard) {
	vector<int> allowedMoves;

	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			int piece = thisBoard[i][j];
			if (getMove(piece, thisBoard) != INVALID) {
				allowedMoves.push_back(piece);
			}
		}
	}
	return allowedMoves;
}

// get a copy of the puzzle/board
Instance getCopy(int** thisBoard, int* thisLastMove, vector<int> thisPath) {
	Instance newInstance;
	int** newPuzzle = new int*[dimension];

	// fill the new Puzzle
	for (int i = 0; i < dimension; i++)
	{
		newPuzzle[i] = new int[dimension];
		for (int j = 0; j < dimension; j++)
		{
			newPuzzle[i][j] = thisBoard[i][j];
		}
	}

	vector<int> newPath;
	for (int i = 0; i < thisPath.size(); i++)
	{
		newPath.push_back(thisPath[i]);
	}

	newInstance.board = newPuzzle;
	newInstance.path = newPath;

	*(newInstance.lastMove) = *thisLastMove;

	return newInstance;
}

vector<int> copyPath(vector<int> copyPath) {
	vector<int> newPath;
	for (int i = 0; i < copyPath.size(); i++)
	{
		newPath.push_back(copyPath[i]);
	}
	return newPath;
}

vector<Instance> visit(int** thisBoard, int* thisLastMove, vector<int> thisPath) {
	vector<Instance> children;
	vector<int> allowedMoves = getAllowedMoves(thisBoard);
	for (int i = 0; i < allowedMoves.size(); i++)
	{
		int newMove = allowedMoves[i];
		if (newMove != *thisLastMove) {
			Instance newInstance = getCopy(thisBoard, thisLastMove, thisPath);
			move(newMove, newInstance.board, newInstance.lastMove);
			newInstance.path.push_back(newMove);
			children.push_back(newInstance);
		}
	}
	return children;
}

// gets the distance between each piece and combines it to get a total number. Higher = further away. 0 = solved
int getDistance(int** thisBoard) {
	int totalDist = 0;
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			int piece = thisBoard[i][j];
			if (piece != 0) {
				int originalLine = (int)floor((piece - 1) / dimension);
				int originalColumn = (int)((piece - 1) % dimension);
				int xDisplace = abs(i - originalLine);
				int yDisplace = abs(j - originalColumn);
				totalDist += xDisplace + yDisplace; // add x and y displacement
			}
		}
	}
	return totalDist;
}


vector<int> solveBFS() {
	Instance startingState = getCopy(board, &mainlastMove, finalPath);
	startingState.path.clear();
	vector<Instance> states = { startingState };

	while (states.size() > 0) {
		Instance state = states[0];

		states.erase(states.begin());

		if (isGoalState(state.board)) {
			// Get a copy of return state, because we need to clean up the rest of the states and delete our memory, but we dont wanna corrupt our good state
			vector<int> returnPath = copyPath(state.path);

			// delete the rest of the states
			for (int i = 0; i < states.size(); i++)
			{
				deleteInstance(states[i]);
			}

			// delete the final state
			deleteInstance(state);

			// return our final path
			return returnPath;
		}

		// append all the "toVisit" instances to the end of states
		vector<Instance> toVisit = visit(state.board, state.lastMove, state.path);
		for (int i = 0; i < toVisit.size(); i++)
		{
			states.push_back(toVisit[i]);
		}

		// if you want to see the program crunching solutions, uncomment the next line
		// printBoard(state.board);

		deleteInstance(state);
	}
}

int lastShortestDist = -1;

vector<int> solveA() {
	Instance startingState = getCopy(board, &mainlastMove, finalPath);
	startingState.path.clear();
	vector<Instance> states = { startingState };

	while (states.size() > 0) {
		// find the closest state
		Instance state = states[0]; // default value
		int shortestDist = -1; // not set value
		int stateIndex = 0; // to keep track of which instance we want to erase

							// get shortest dist
		for (int i = states.size() - 1; i > 0; i--) // theoritically since you push to the end, the ones toward the end are more likely to be closer to the correct path, so we should check those first. Also this is what makes it essentially depth first. We basically read the vector backwards, so we push and pop from the 'front' essentially
		{
			int newDist = getDistance(states[i].board) + states[i].path.size(); // distance from the final state, plus the length of the current path
			if (shortestDist == -1 || newDist < shortestDist) {
				// new shortest distance
				shortestDist = newDist;
				state = states[i];
				stateIndex = i;
				if (lastShortestDist == -1 || newDist < lastShortestDist)
					break; // we found one closer. The closest we can get is by one move, so lets just go with the first one we find that is closer
			}
		}
		// now state is now the instance that is most correct

		// update the lastShortestDist to whatever dist we just found
		lastShortestDist = shortestDist;

		states.erase(states.begin() + stateIndex);

		if (isGoalState(state.board)) {
			// Get a copy of return state, because we need to clean up the rest of the states and delete our memory, but we dont wanna corrupt our good state
			vector<int> returnPath = copyPath(state.path);

			// delete the rest of the states
			for (int i = 0; i < states.size(); i++)
			{
				deleteInstance(states[i]);
			}

			// delete the final state
			deleteInstance(state);

			// return our final path
			return returnPath;
		}

		// append all the "toVisit" instances to the end of states
		vector<Instance> toVisit = visit(state.board, state.lastMove, state.path);
		for (int i = 0; i < toVisit.size(); i++)
		{
			states.push_back(toVisit[i]);
		}

		// if you want to see the program crunching solutions, uncomment the next line
		// printBoard(state.board);

		deleteInstance(state); // delete the instance that we just checked
	}
}

void shuffle(int density) {
	for (int i = 0; i < density; i++)
	{
		vector<int> allowedMoves = getAllowedMoves(board);

		if (allowedMoves.size() > 1)
		{
			// choose a random number to move
			int randomNum = rand() % allowedMoves.size();

			int newMove = allowedMoves[randomNum];
			move(newMove, board, &mainlastMove);
		}
	}
	// reset last move
	mainlastMove = -1;
}

void printBoard(int** thisBoard) {
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			// print it
			cout << setw(4) << thisBoard[i][j];
		}
		cout << endl << endl;
	}
	cout << "-----------------------------" << endl << endl;
}

void solveBack(int** thisBoard, vector<int> moves) {
	// loop through each direction and apply it to the initial board
	int* noMove = new int;
	*noMove = -1;
	for (int i = 0; i < moves.size(); i++)
	{
		cin.get();
		move(moves[i], thisBoard, noMove);
		printBoard(thisBoard);
	}
	delete noMove;
}

void deleteBoard(int** thisBoard) {
	for (int i = 0; i < dimension; i++)
	{
		delete[] thisBoard[i];
	}

	delete[] thisBoard;
}

void deleteInstance(Instance thisInstance) {
	deleteBoard(thisInstance.board);
	delete thisInstance.lastMove;
	thisInstance.path.clear();
}

void runProgram() {
	// set up the puzzle
	cout << "Enter dimensions for the sliding puzzle (single int value) 4 recommended" << endl;
	int dim = 4;
	cin >> dim;
	setupPuzzle(dim);

	cout << endl << "Puzzle set. Please enter number of times you wish to shuffle the board (single int value, recommended no more than 100)" << endl;
	int shuff = 4;
	cin >> shuff;
	shuffle(shuff);

	cout << "-----------------------------" << endl << endl;
	printBoard(board);

	cout << endl << "SHUFFLED. Type A for A* search, or B for breadth first search" << endl;
	char response = 'b';
	cin >> response;

	while (response != 'A' && response != 'a' && response != 'B' && response != 'b')
		cin >> response;

	vector<int> moves;

	if (response == 'A' || response == 'a') {
		cout << "Solving..." << endl << endl;
		moves = solveA();
	}
	else if (response == 'B' || response == 'b') {
		cout << "Solving..." << endl << endl;
		moves = solveBFS();
	}
	else {
		cout << "Invalid input. Please restart program" << endl;
		return;
	}

	cout << "-----SOLVED-----" << endl << endl << "Press Enter to show each step" << endl << endl << endl;
	solveBack(board, moves);
	cout << "-----DONE WITH SOLUTION-----" << endl;

	cin.get(); // pause at the end
}

int main()
{
	bool programRunning = true;
	while (programRunning == true) {
		runProgram();
		cout << endl << "Enter Q to quit or anything else to restart" << endl;
		char input;
		cin >> input;
		if (input == 'q' || input == 'Q') {
			programRunning = false;
		}
	}


	return 0;
}