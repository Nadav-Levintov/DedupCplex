#!/bin/bash
CSV=".csv"
LOG=".log"
SIZE="4"
KB="1024"
MB=$(( KB * KB ))
TOTALS=$(( SIZE * MB ))
K="0"
EPSILON="0"
OUTFILE=""
KBTGB=$MB
GZ=".gz"
FILENAME=""
function getMinSysNum{
	MINSYS=`cat $FILENAME | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort | tail -1`
}

function getMaxSysNum{
	MAXSYS=`cat $FILENAME | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort | head -1`
}

function getContainers{
	CONTAINERS=`cat $FILENAME | grep "#" | grep "containers" -i | grep "num" -i | cut -f2 -d":"`
}

function getBlocks{
	BLOCKS=`cat $FILENAME | grep "#" | grep "blocks" -i | grep "num" -i | cut -f2 -d":"`
}

function getFiles{
	FILES=`cat $FILENAME | grep "#" | grep "files" -i | grep "num" -i | cut -f2 -d":"`
}

function getNumSystem{
	SYSTEM=`cat $FILENAME  | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | uniq | wc -l`
}

function physicalOrLogical{
	if [[ $FILENAME =~ .*_b_.* ]] || [[ $FILENAME =~ .*_B_.* ]]
	then
		LOGICAL=$FILES
		PHYSICAL="0"
	else
		PHYSICAL=$FILES
		LOGICAL="0"
	fi
}


