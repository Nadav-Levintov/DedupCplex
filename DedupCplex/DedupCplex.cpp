// DedupCplex.cpp : Defines the entry point for the console application.
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ParserSolver.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <cmath>
#include "ParserSolver.h"

#define CPLEX_TIME_LIMIT_IN_SECONDS (12*60*60) //12 hours

ParserSolver::ParserSolver(string& filename, string& K, string &eps) {
	IloEnv env;
	bool isContainers = false;

	string::size_type sz;
	uint32_t i;
	this->fileName = filename;
	ifstream file(filename);
	
	this->numK = K;
	this->epsilon = eps;
	string st;	// reads line after line from inputFile
	
    
	timeInput = 0;
	

	try {
		IloNum start;
		IloModel model(env);
		IloNumVarArray vars_c(env);
		IloNumVarArray vars_m(env);	
		IloNumVarArray vars_f(env);	
		IloRangeArray con(env);
		IloCplex cplex(model);
		IloExpr blockSizeCopy(env);
		IloExpr blockSizeMove(env);
		cplex.setParam(IloCplex::Param::TimeLimit, CPLEX_TIME_LIMIT_IN_SECONDS);

		start = cplex.getTime();
		
		if (!file.is_open())
		{
				cout << "failed to open file" << endl;
				cplex.end();
				return;
		}
			
		getline(file, st);

		while (st.at(0) == '#') {	// read the header for info
			vector<string> t = string_split(st, " ");

			// find number of files and blocks in header
			if (t.size() >= 4) {
				if (!t[2].compare("files:") && t[3].compare("")) {		// # Num Files: <nFiles>
					this->nFiles = stoi(t[3]);
				}
				if (!t[2].compare("blocks:")){	// # Num Blocks: <nblocks>
					this->nBlocks = stoi(t[3]);
				}
				if (!t[2].compare("Blocks:")){	// # Num Blocks: <nblocks>
					this->nBlocks = stoi(t[3]);
				}
				if (!t[2].compare("physical")) {	// # Num physical: <nblocks>
					this->nBlocks = stoi(t[4]);
				}
				if (!t[2].compare("containers")) {	// # Num Containers: <nblocks>
					this->nBlocks = stoi(t[4]);
					isContainers = true;
				}
				
			}
				getline(file, st);
		}
		vector<int> blocks;	// initialize blocks array, to mark which block has been read yet and to save their sizes
		vector<bool> files;	// initialize blocks array, to mark which block has been read yet and to save their sizes
		
		// -1 = has not been read yet
		for (i = 0; i < this->nBlocks; i++)
			blocks.push_back(-1);
		
		// false = empty file or not read yet
		for (i = 0; i < this->nFiles; i++)
			files.push_back(false);
		
		
		for (i = 0; i < this->nBlocks; i++)
		{
			string c_name = "c" + to_string(i);
			string m_name = "m" + to_string(i);

			vars_c.add(IloNumVar(env, 0, 1, ILOINT, c_name.c_str()));// 0 <= c[i] <= 1
			vars_m.add(IloNumVar(env, 0, 1, ILOINT, m_name.c_str()));// 0 <= m[i] <= 1
			inputSize += 2;
	
			model.add(vars_c[i] + vars_m[i]<= 1);

		}
		while (st.at(0) == 'F') 
		{	// while at files list

			// get sizes of blocks and add to total, and add their term to the cplex formula
			vector<string> fTemp = string_split(st, ","); //string_split into chunks between ','

			// [0]F, [1]file id, [2]file name, [3]directory, [4]num of blocks, [5+2i]block i id, [6+2i]block i size

			for (i = 5; i < (fTemp.size()-1); i = i + 2) {
				int blockSn = stoi(fTemp[i]);	// add block id to list of file blocks
				files[ceil(stod(fTemp[1], &sz))] = true;
				if (blocks[blockSn] == -1) {	// block hasn't been seen yet in list of files								
					double temp = 1;		//in case containers we count their size as equal (1KB)
					if(!isContainers)
					{
						temp = fmax(1,ceil(stod(fTemp[i + 1], &sz) / 1024));	// block size in kb
					}
					
					blocks[blockSn] = (int)(temp);
					this->totalSize += blocks[blockSn];
					blockSizeCopy +=  blocks[blockSn]*vars_c[blockSn];// c[i]*size[i]
					blockSizeMove += blocks[blockSn]*vars_m[blockSn];// m[i]*size[i]
					inputSize += 2;
				}
			}

			getline(file, st);
		}// End while(F)
			
		for (i = 0; i < nFiles; i++)
		{
			string f_name = "f" + to_string(i);
			vars_f.add(IloNumVar(env, 0, 1, ILOINT, f_name.c_str()));// 0 <= f[i] <= 1 
			inputSize++;	
		}
		
		while (st.at(0) == 'B' || st.at(0) == 'P' || st.at(0) == 'C') 
		{	// while at blocks/phsysical files list	
			vector<string> bTemp = string_split(st, ",");
			// [0]B/P, [1]block_sn, [2]block name, [3]num of files containing the block, [4]file id ...[i]file id
			// [0]C, [1]container_sn, [2]container size, [3]num of files containing the container, [4]file id ...[i]file id

			int bsn = stoi(bTemp[1]);
			for (i = 4; i < (bTemp.size()-1); i++) { 	// list of file sns that the block is contained in
				int fsn = stoi(bTemp[i]);

				model.add( vars_f[fsn] - vars_m[bsn] >= 0 );					// mi <= fj 	
				model.add( vars_m[bsn] + vars_c[bsn] - vars_f[fsn] >= 0 );		// fj <= mi+ci

				inputSize++;
			}

			if (!getline(file, st)) 
			{
				break;
			}
		}// End if(B)
						
		int onePercent;
		int lowerbound, upperbound;

		// Calculate K, eps
		if (K.at(K.length() - 1) == '%') {	//input asked for values in %
			onePercent = totalSize / 100;
			K = K.substr(0, K.length() - 1);
			targetMove = onePercent * stol(K);
			this->epsilon = (eps.length() - 1) < 5 ? eps.substr(0, epsilon.length() - 1) : eps.substr(0, 5);
			targetEpsilon = (long)(onePercent*stod(this->epsilon, &sz));
		}
		else	// input asked in absolute values
		{
			targetMove = stol(K);
			targetEpsilon = stol(eps);
		}
		
		// constraints: kTemp+epsTemp <= blockSizeMove <= kTemp+epsTemp
		lowerbound = targetMove - targetEpsilon;
		upperbound = targetMove + targetEpsilon;
		model.add(blockSizeMove >= lowerbound);
		model.add(blockSizeMove <= upperbound);
		inputSize += 2;
		

		model.add(IloMinimize(env, blockSizeCopy));
		blockSizeCopy.end();
		cplex.exportModel ("lpex1.lp");
		this->timeInput = cplex.getTime() - start;
		start = cplex.getTime();
		if (cplex.solve() == IloTrue) 
		{
			this->time = cplex.getTime() - start;
			//	Count and mark the files that move		
			IloNumArray vals_f(env);
			
			for (IloInt i = 0; i <this->nFiles; i++)
			{
				if (files[i])
				{
					if (1 == cplex.getValue(vars_f[i]))
					{
						numOfMoveFiles++;
						this->moveFile.push_back(i);
					}
				}
			}
			
			//env.out() << "Values f = " << vals_f << endl;

			IloNumArray vals_m(env);
			cplex.getValues(vals_m, vars_m);
			
			IloNumArray vals_c(env);
			cplex.getValues(vals_c, vars_c);
			
			for (i = 0; i < this->nBlocks; i++) 
			{
				//	Count and mark the blocks that are moved + their size

				if (vals_m[i] == 1) {
					totalMoveSpace += blocks[i];
					numOfMoveBlocks++;
					moveBlock.push_back(i);
				}
				//	Count and mark the blocks that are copied + their size

				if (vals_c[i] == 1) 
				{
					totalCopySpace += blocks[i];
					numOfCopyBlocks++;
					copyBlock.push_back(i);
				}
			}
			//env.out() << "Values m = " << vals_m << endl;
			//env.out() << "Values c = " << vals_c << endl;
		}
		else {
			cout << "Bad file!\n Problem not solved." << endl;
			exit(0);
		}
	//cplex.exportModel ("lpex1.lp");

	}
	catch (IloException e) { // end cplex try
		cout << e.getMessage() << endl;
	}// End try

}

// get functions

list<string>& ParserSolver::getOutput() {
	return this->output;
}


string& ParserSolver::getFileName() {
	return this->fileName;
}

double ParserSolver::getTime() {
	return this->time;
}

double ParserSolver::getTimeInput() {
	return this->timeInput;
}

long ParserSolver::getTotalMoveSpace() {
	return this->totalMoveSpace;
}

long ParserSolver::getTotalCopySpace() {
	return this->totalCopySpace;
}

int ParserSolver::getNumOfMoveFiles() {
	return this->numOfMoveFiles;
}

int ParserSolver::getNumOfFiles() {
	return this->nFiles;
}

int ParserSolver::getNumOfBlocks() {
	return this->nBlocks;
}

int ParserSolver::getNumOfMoveBlocks() {
	return this->numOfMoveBlocks;
}

int ParserSolver::getNumOfCopyBlocks() {
	return this->numOfCopyBlocks;
}


list<int>& ParserSolver::getMoveFile() {
	return this->moveFile;
}

list<int>& ParserSolver::getMoveBlock() {
	return this->moveBlock;
}

list<int>& ParserSolver::getCopyBlock() {
	return this->copyBlock;
}

long ParserSolver::getTotalSize() {
	return this->totalSize;
}

int ParserSolver::getInputSize() {
	return this->inputSize;
}

string& ParserSolver::getNumK() {
	return this->numK;
}

long ParserSolver::getTargetMove() {
	return this->targetMove;
}

long ParserSolver::getTargetEpsilon() {
	return this->targetEpsilon;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		cout << "usage: filename k epsilon \n";
		return 0;
	}

	string fileName, k, epsilon;
	// boolean obj;

	fileName = string(argv[1]);
	// get all file name parts: path parts, name and extension
	vector<string> fileParts = string_split(fileName,"\\.");
	vector<string> nameParts = string_split(fileParts[0],"\\/");

	k = string(argv[2]);
	epsilon = string(argv[3]);

	// parse input and run solver
	ParserSolver solve(fileName, k, epsilon);

	// prepare output file name: <original_filename>_<k>_<epsilon>_result.csv
	string outputFileName("");
	for (uint32_t i = 0; i < nameParts.size() - 1; i++) {
		outputFileName += nameParts[i];
		outputFileName += "/";
	}
	//outputFileName += "output/";
	outputFileName += nameParts[nameParts.size() - 1];
	outputFileName += "_" + k + "_" + epsilon + "_result.csv";

	ofstream  exportFile;
	exportFile.open(outputFileName);

	// write output data from solver to output file
		if (exportFile.good()) {

			exportFile << "Files: " << solve.getNumOfFiles() << endl;
			exportFile << "Blocks: " << solve.getNumOfBlocks()  << endl;
			exportFile << "Target Move: " << solve.getTargetMove() << endl;
			exportFile << "Epsilon: " << solve.getTargetEpsilon()  << endl;
			exportFile << "Input time: " << solve.getTimeInput()  << endl;
			exportFile << "Input Size: " << solve.getInputSize()  << endl;
			exportFile << "Solve Time: " << solve.getTime() << endl;
			exportFile << "RAM:" << endl;
			exportFile << "Moved Storage: " << solve.getTotalMoveSpace()  << endl;
			exportFile << "Copied Storage: " <<solve.getTotalCopySpace()  << endl;
			exportFile << "Moved Files: " << solve.getNumOfMoveFiles()  << endl;
			exportFile << "Moved Blocks: " << solve.getNumOfMoveBlocks() << endl;
			exportFile << "Copied Blocks: " << solve.getNumOfCopyBlocks() << endl;
			exportFile << "Total Size: " << solve.getTotalSize() << endl;

			cout << "File " << outputFileName << " has been created!" << endl;
		}
		else
		{
			cout << "File" << outputFileName << " already exists." << endl;
		}

	exportFile.close();

	return 0;
}
