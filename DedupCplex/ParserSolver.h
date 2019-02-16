#pragma once
#ifndef PARSER_SOLVER_H
#define PARSER_SOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include "ilcplex/ilocplex.h"

using namespace std;

vector<string> string_split(string str, string token) {
	vector<string>result;
	while (str.size()) {
		int index = str.find(token);
		if (index != string::npos) {
			result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
			if (str.size() == 0)result.push_back(str);
		}
		else {
			result.push_back(str);
			str = "";
		}
	}
	return result;
}

class ParserSolver {

	string fileName;
	list<string> output;				// Save the relevant information about the file system (to the export file).
	double time = 0;													// execution time.
	double timeInput = 0;												// duration of creating input to cplex (files&blocks arrays).
	IloCplex cplex;
	double totalMoveSpace = 0;										// total space that is moved 
	double totalCopySpace = 0;										// total space that is copied 
	int numOfMoveFiles = 0;											// number of files to be moved 
	int numOfMoveBlocks = 0, numOfCopyBlocks = 0;					// number of blocks to be copied 
	list<int> moveFile;			// Array of files to be moved
	list<int> moveBlock;		// Array of blocks to be moved
	list<int> copyBlock;		// Array of blocks to be copied
	double totalSize = 0;												// Total system size
	int inputSize = 0;												// size of the files&blocks arrays.
	string numK;
	string epsilon;
	int targetMove;
	int targetEpsilon;
	int nFiles = 0;													// number of input files
	int nBlocks = 0;												// number of input blocks

public:
	ParserSolver(string& filename, string& K, string& eps);

	// get functions

	list<string>& getOutput();


	string& getFileName();
	double getTime();
	double getTimeInput();
	long getTotalMoveSpace();
	long getTotalCopySpace();
	int getNumOfMoveFiles();
	int getNumOfFiles();
	int getNumOfBlocks();
	int getNumOfMoveBlocks();
	int getNumOfCopyBlocks();
	list<int>& getMoveFile();
	list<int>& getMoveBlock();
	list<int>& getCopyBlock();
	long getTotalSize();
	int getInputSize();
	string& getNumK();
	long getTargetMove();
	long getTargetEpsilon();
};

#endif // !PARSER_SOLVER_H