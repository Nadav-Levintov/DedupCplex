#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <cmath>
#include "ParserSolver.h"


ParserSolver::ParserSolver(string filename, string K, string eps) {
	IloEnv env;
	
	string::size_type sz;
	uint32_t i;
	this->fileName = filename;
	ifstream file(filename);
	
	
	/*
	long startTime = System.nanoTime();
	long duration = System.nanoTime() - startTime;
	timeInput = duration/1000000000.0;
	*/

	this->numK = K;
	this->epsilon = eps;
	string st;	// reads line after line from inputFile


	try {
		IloModel model(env);
		IloRangeArray constraints(env);

		IloExpr blockSizeCopy(env);	// Sigma(size(i)*c(i))
		IloExpr blockSizeMove(env);	// Sigma(size(i)*m(i))

		if(file.is_open()) {	// set up the solver
			getline(file, st);

			while (st.at(0) == '#') {	// read the header for info
				vector<string> t = string_split(st," ");

				// find number of files and blocks in header
				if (t.size() >= 4) {
					if (!t[2].compare("files:")) {		// # Num Files: <nFiles>
						this->nFiles = stoi(t[3]);
					}
					if (!t[2].compare("blocks:")) {	// # Num Blocks: <nblocks>
						this->nBlocks = stoi(t[3]);
					}
				}
				getline(file, st);
			}

			// array of cplex variables for blocks to be moved
			IloNumVarArray m(env);
			// array of cplex varialbes for blocks to be copied
			IloNumVarArray c(env);

			vector<int> blocks;	// initialize blocks array, to mark which block has been read yet and to save their sizes

												// -1 = has not been read yet
			for (i = 0; i < this->nBlocks; i++)
				blocks.push_back(-1);

			for (i = 0; i < this->nBlocks; i++)
			{
				//x.add(IloNumVar(env, 0.0, 40.0,        "x0"));
				string c_name = "c" + to_string(i);
				string m_name = "m" + to_string(i);

				c.add(IloNumVar(env,0,1,c_name.c_str()));// 0 <= c[i] <= 1
				m.add(IloNumVar(env, 0, 1, m_name.c_str()));// 0 <= m[i] <= 1
				inputSize += 2;

				constraints.add(c[i] + m[i] <= 1);// c[i]+m[i] <= 1 	
			}

			while (st.at(0) == 'F') {	// while at files list

											// get sizes of blocks and add to total, and add their term to the cplex formula
				vector<string> fTemp = string_split(st, ","); //string_split into chunks between ','

													// [0]F, [1]file id, [2]file name, [3]directory, [4]num of blocks, [5+2i]block i id, [6+2i]block i size

				for (i = 5; i < fTemp.size(); i = i + 2) {
					int blockSn = stoi(fTemp[i]);	// add block id to list of file blocks

					if (blocks[blockSn] == -1) {	// block hasn't been seen yet in list of files								
						double temp = ceil(stod(fTemp[i + 1],&sz) / 1024);	// block size in kb
						blocks[blockSn] = (int)(temp);
						totalSize += blocks[blockSn];
						blockSizeCopy += c[blockSn] * blocks[blockSn];// c[i]*size[i]
						blockSizeMove += m[blockSn] * blocks[blockSn];// m[i]*size[i]
						inputSize += 2;
					}
				}

				getline(file, st);
			}// End while(F)

			IloNumVarArray f(env);	// initialize files list
			for (i = 0; i < nFiles; i++)
			{
				string f_name = "f" + to_string(i);
				f.add(IloNumVar(env, 0, 1, f_name.c_str()));// 0 <= f[i] <= 1 

				constraints.add(0 <= f[i]);// 0 <= f[i] 	
				constraints.add(f[i] <= 1);// f[i] <= 1 	
				inputSize++;
			}

			while (st.at(0) == 'B' || st.at(0) == 'P') {	// while at blocks/phsysical files list	
				vector<string> bTemp = string_split(st, ",");
				// [0]F, [1]file id, [2]file name, [3]directory, [4]num of blocks, [5+2i]block i id, [6+2i]block i size

				int bsn = stoi(bTemp[1]);
				for (i = 4; i < bTemp.size(); i++) { 	// list of file sns that the block is contained in
					int fsn = stoi(bTemp[i]);

					IloNum m_val = m[bsn].getNumConstant();
					IloNum f_val = f[fsn].getNumConstant();

					//TODO: cannot add constraints on two vars, one must be constant
					// Couldn't find answer, not sure my solution works
					/*
					constraints.add(m[bsn] <= f[fsn]);// mi <= fj 	
					constraints.add(f[fsn] <= m[bsn] + c[bsn]);// fj <= mi+ci
					*/
					constraints.add(m_val <= f[fsn]);// mi <= fj 	
					constraints.add(f_val <= m[bsn] + c[bsn]);// fj <= mi+ci

					inputSize++;
				}

				getline(file, st);
				if (st.length() == 0) break;
			}// End if(B)

			IloObjective obj = IloMinimize(env, blockSizeCopy);
			model.add(obj);

			long onePercent;

			long lowerbound, upperbound;
			// Calculate K, eps
			if (K.at(K.length() - 1) == '%') {	//input asked for values in %
				onePercent = totalSize / 100;
				K = K.substr(0, K.length() - 1);
				targetMove = onePercent * stol(K);
				eps = epsilon.length() - 1 < 5 ? epsilon.substr(0, epsilon.length() - 1) : epsilon.substr(0, 5);
				targetEpsilon = (long)(onePercent*stod(eps,&sz));
			}
			else	// input asked in absolute values
			{
				targetMove = stol(K);
				targetEpsilon = stol(eps);
			}

			// constraints: kTemp+epsTemp <= blockSizeMove <= kTemp+epsTemp
			lowerbound = targetMove - targetEpsilon;
			upperbound = targetMove + targetEpsilon;
			constraints.add(lowerbound <= blockSizeMove);
			constraints.add(blockSizeMove <= upperbound);
			inputSize += 2;

			model.add(constraints);

			if (cplex.solve()) {	//run silver

									//	Count and mark the files that move
				int f_size = f.getSize();
				for (i = 0; i < f_size; i++) {
					if (cplex.getValue(f[i]) == 1) {
						// files.get(i).setDelete(1);
						numOfMoveFiles++;
						this->moveFile.push_back(i);
					}
				}

				int m_size = m.getSize();
				for (i = 0; i < m_size; i++) {
					//	Count and mark the blocks that are moved + their size
					if (cplex.getValue(m[i]) == 1) {
						totalMoveSpace += blocks[i];
						numOfMoveBlocks++;
						moveBlock.push_back(i);
					}
					//	Count and mark the blocks that are copied + their size
					if (cplex.getValue(c[i]) == 1) {
						totalCopySpace += blocks[i];
						numOfCopyBlocks++;
						copyBlock.push_back(i);
					}
				}

				time = cplex.getCplexTime();

			}
			else {
				cout << "Bad file!\n Problem not solved." << endl;
				exit(0);
			}


		}
		else { // end readfile try
			cout << "failed to open file" << endl;
		}
		time = cplex.getCplexTime();
		cplex.end();
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