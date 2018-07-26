// DedupCplex.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ilcplex/ilocplexi.h"

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("usage: filename k epsilon \n");
		exit(0);
	}

	IloEnv   env;
	try {
		IloModel model(env);

		if ((argc != 2) ||
			(argv[1][0] != '-') ||
			(strchr("rcn", argv[1][1]) == NULL)) {
			usage(argv[0]);
			throw(-1);
		}

		IloNumVarArray var(env);
		IloRangeArray con(env);

		switch (argv[1][1]) {
		case 'r':
			populatebyrow(model, var, con);
			break;
		case 'c':
			populatebycolumn(model, var, con);
			break;
		case 'n':
			populatebynonzero(model, var, con);
			break;
		}

		IloCplex cplex(model);
	}

	ParserSolver solve = new ParserSolver();
	string fileName, k, epsilon;
	// boolean obj;

	fileName = args[0];
	// get all file name parts: path parts, name and extension
	string[] fileParts = fileName.split("\\.");
	string[] nameParts = fileParts[0].split("\\/");

	k = args[1];
	epsilon = args[2];

	// parse input and run solver
	solve.parseAndSolve(fileName, k, epsilon);

	// prepare output file name: <original_filename>_<k>_<epsilon>_result.csv
	String outputFileName = "";
	for (int i = 0; i < nameParts.length - 1; i++) {
		outputFileName += nameParts[i];
		outputFileName += "/";
	}
	outputFileName += "output/";
	outputFileName += nameParts[nameParts.length - 1];
	outputFileName += "_" + k + "_" + epsilon + "_result.csv";

	File exportFile = new File(outputFileName);

	// write output data from solver to output file
	try {

		if (exportFile.createNewFile()) {
			BufferedWriter writer = new BufferedWriter(new FileWriter(exportFile));

			/* output for Maor's input -- ignore
			writer.write(solve.getNumOfFS() + ",");
			writer.write(solve.getFirstFS() + ",");
			writer.write(solve.getLastFS() + ",");
			writer.write(solve.getTarget() + ",");
			writer.write(solve.getHeuristics() + ",");
			*/

			writer.write(solve.getNumOfFiles() + ",");
			writer.write(solve.getNumOfBlocks() + ",");
			writer.write(solve.getTargetMove() + ",");
			writer.write(solve.getTargetEpsilon() + ",");
			writer.write(solve.getTimeInput() + ",");
			writer.write(solve.getInputSize() + ",");
			writer.write(solve.getTime() + ",");
			writer.write("RAM=,");
			writer.write(solve.getTotalMoveSpace() + ",");
			writer.write(solve.getTotalCopySpace() + ",");
			writer.write(solve.getNumOfFiles() + ",");
			writer.write(solve.getNumOfMoveBlocks() + ",");
			writer.write(solve.getNumOfCopyBlocks() + ",");

			writer.close();
			System.out.println("File " + outputFileName + " has been created!");
		}
	}
	catch (IOException e1) {
		System.out.println("File" + outputFileName + " already exists.");
	}
	return 0;
}

