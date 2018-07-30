// DedupCplex.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ParserSolver.h"
#include "ilcplex/ilocplexi.h"


int main(int argc, char *argv[])
{
	if (argc < 3) {
		cout << "usage: filename k epsilon \n";
		return 0;
	}

	string fileName, k, epsilon;
	// boolean obj;

	fileName = string(argv[0]);
	// get all file name parts: path parts, name and extension
	vector<string> fileParts = string_split(fileName,"\\.");
	vector<string> nameParts = string_split(fileParts[0],"\\/");

	k = string(argv[1]);
	epsilon = string(argv[2]);

	// parse input and run solver
	ParserSolver solve(fileName, k, epsilon);

	// prepare output file name: <original_filename>_<k>_<epsilon>_result.csv
	string outputFileName("");
	for (uint32_t i = 0; i < nameParts.size() - 1; i++) {
		outputFileName += nameParts[i];
		outputFileName += "/";
	}
	outputFileName += "output/";
	outputFileName += nameParts[nameParts.size() - 1];
	outputFileName += "_" + k + "_" + epsilon + "_result.csv";



	ofstream  exportFile;
	exportFile.open(outputFileName);

	// write output data from solver to output file
		if (exportFile.good()) {

			exportFile << solve.getNumOfFiles() << "," << endl;
			exportFile << solve.getNumOfBlocks() << "," << endl;
			exportFile << solve.getTargetMove() << "," << endl;
			exportFile << solve.getTargetEpsilon() << "," << endl;
			exportFile << solve.getTimeInput() << "," << endl;
			exportFile << solve.getInputSize() << "," << endl;
			exportFile << solve.getTime() << "," << endl;
			exportFile << "RAM=," << endl;
			exportFile << solve.getTotalMoveSpace() << "," << endl;
			exportFile << solve.getTotalCopySpace() << "," << endl;
			exportFile << solve.getNumOfFiles() << "," << endl;
			exportFile << solve.getNumOfMoveBlocks() << "," << endl;
			exportFile << solve.getNumOfCopyBlocks() << "," << endl;

			cout << "File " << outputFileName << " has been created!" << endl;
		}
		else
		{
			cout << "File" << outputFileName << " already exists." << endl;
		}

	exportFile.close();

	return 0;
}

