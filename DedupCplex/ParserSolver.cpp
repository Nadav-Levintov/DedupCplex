#pragma once
#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include "ParserSolver.h"

using namespace std;

vector<string> split(string target, string delim)
{
	vector<string> v;
	if (!target.empty()) {
		string::size_type start = 0;
		do {
			size_t x = target.find(delim, start);
			if (x == string::npos)
				break;

			v.push_back(target.substr(start, x - start));
			start += delim.size();
		} while (true);

		v.push_back(target.substr(start));
	}
	return v;
}

void ParserSolver::parseAndSolve(string filename, string K, string eps) {
	IloEnv env;
	
	string::size_type sz;
	int i;
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
				vector<string> t = split(st," ");

				// find number of files and blocks in header
				if (t.size >= 4) {
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
			model.add(constraints);

			while (st.at(0) == 'F') {	// while at files list

											// get sizes of blocks and add to total, and add their term to the cplex formula
				vector<string> fTemp = split(st, ","); //split into chunks between ','

													// [0]F, [1]file id, [2]file name, [3]directory, [4]num of blocks, [5+2i]block i id, [6+2i]block i size

				for (i = 5; i < fTemp.size; i = i + 2) {
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
				//TODO: continue from here
				c.add(IloNumVar(env, 0, 1, c_name.c_str()));// 0 <= c[i] <= 1

				f[i] = cplex.numVar(0, 1, IloNumVarType.Int);	// 0 <= f[i] <= 1 
				cplex.addLe(0, f[i]);
				cplex.addLe(f[i], 1);
				inputSize++;
			}

			while (st.at(0) == 'B' || st.at(0) == 'P') {	// while at blocks/phsysical files list	
				string[] bTemp = st.split(",");
				// [0]F, [1]file id, [2]file name, [3]directory, [4]num of blocks, [5+2i]block i id, [6+2i]block i size

				int bsn = stoi(bTemp[1]);
				for (i = 4; i < bTemp.length; i++) { 	// list of file sns that the block is contained in
					int fsn = stoi(bTemp[i]);
					cplex.addLe(m[bsn], f[fsn]);						// mi <= fj
					cplex.addLe(f[fsn], cplex.sum(m[bsn], c[bsn]));		// fj <= mi+ci
					inputSize++;
				}

				st = br.readLine();
				if (st == null) break;
			}// End if(B)


			cplex.addMinimize(blockSizeCopy); // ask cplex to minimize the total size of copied blocks

			long onePercent;

			long lowerbound, upperbound;
			// Calculate K, eps
			if (K.at(K.length() - 1) == '%') {	//input asked for values in %
				onePercent = totalSize / 100;
				K = K.substring(0, K.length() - 1);
				targetMove = onePercent * Long.parseLong(K);
				eps = epsilon.substring(0, min(epsilon.length() - 1, 5));
				targetEpsilon = (long)(onePercent*stod(eps,&sz));
			}
			else	// input asked in absolute values
			{
				targetMove = Long.parseLong(K);
				targetEpsilon = Long.parseLong(eps);
			}

			// constraints: kTemp+epsTemp <= blockSizeMove <= kTemp+epsTemp
			lowerbound = targetMove - targetEpsilon;
			upperbound = targetMove + targetEpsilon;
			cplex.addLe(lowerbound, blockSizeMove);
			cplex.addLe(blockSizeMove, upperbound);
			inputSize += 2;

			if (cplex.solve()) {	//run silver

									//	Count and mark the files that move
				for (i = 0; i < f.length; i++) {
					if (cplex.getValue(f[i]) == 1) {
						// files.get(i).setDelete(1);
						numOfMoveFiles++;
						moveFile.add(i);
					}
				}


				for (i = 0; i < m.length; i++) {
					//	Count and mark the blocks that are moved + their size
					if (cplex.getValue(m[i]) == 1) {
						totalMoveSpace += blocks[i];
						numOfMoveBlocks++;
						moveBlock.add(i);
					}
					//	Count and mark the blocks that are copied + their size
					if (cplex.getValue(c[i]) == 1) {
						totalCopySpace += blocks[i];
						numOfCopyBlocks++;
						copyBlock.add(i);
					}
				}

				time = cplex.getCplexTime();

			}
			else {
				if (JOptionPane.showConfirmDialog(null, "Bad file!\n Problem not solved.", "Error message", JOptionPane.DEFAULT_OPTION, JOptionPane.ERROR_MESSAGE) == 0)
					System.exit(0);
			}


		}
		else { // end readfile try
			cout << "failed to open file" << endl;
		}
		time = cplex.getCplexTime();
		cplex.end();
	}
	catch (IloException e) { // end cplex try
		e.printStackTrace();
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