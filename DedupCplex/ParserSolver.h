#pragma once
#ifndef PARSER_SOLVER_H
#define PARSER_SOLVER_H

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include "ilcplex/ilocplexi.h"

using namespace std;

class ParserSolver {

	string fileName;
	list<string> output;				// Save the relevant information about the file system (to the export file).
	double time = 0;													// execution time.
	double timeInput = 0;												// duration of creating input to cplex (files&blocks arrays).
	IloCplex cplex;
	long totalMoveSpace = 0;										// total space that is moved 
	long totalCopySpace = 0;										// total space that is copied 
	int numOfMoveFiles = 0;											// number of files to be moved 
	int numOfMoveBlocks = 0, numOfCopyBlocks = 0;					// number of blocks to be copied 
	list<int> moveFile;			// Array of files to be moved
	list<int> moveBlock;		// Array of blocks to be moved
	list<int> copyBlock;		// Array of blocks to be copied
	long totalSize = 0;												// Total system size
	int inputSize = 0;												// size of the files&blocks arrays.
	string numK;
	string epsilon;
	long targetMove;
	long targetEpsilon;
	int nFiles = 0;													// number of input files
	int nBlocks = 0;												// number of input blocks

	void parseAndSolve(string filename, string K, string eps);

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
}

#endif // !PARSER_SOLVER_H