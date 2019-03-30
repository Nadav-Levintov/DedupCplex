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

function getMinSysNum {
	MINSYS=`cat ${FILEPATH} | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort | tail -1`
}

function getMaxSysNum {
	MAXSYS=`cat ${FILEPATH} | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort | head -1`
}

function getContainers {
	CONTAINERS=`cat ${FILEPATH} | grep "#" | grep "containers" -i | grep "num" -i | cut -f2 -d":"`
}

function getBlocks {
	BLOCKS=`cat ${FILEPATH} | grep "#" | grep "blocks" -i | grep "num" -i | cut -f2 -d":"`
}

function getFiles {
	FILES=`cat ${FILEPATH} | grep "#" | grep "files" -i | grep "num" -i | cut -f2 -d":"`
}

function getNumSystem {
	SYSTEM=`cat ${FILEPATH}  | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | uniq | wc -l`
}

function physicalOrLogical {
	if [[ ${FILEPATH} =~ .*_b_.* ]] || [[ ${FILEPATH} =~ .*_B_.* ]]
	then
		LOGICAL=$FILES
		PHYSICAL="0"
	else
		PHYSICAL=$FILES
		LOGICAL="0"
	fi
}

RUNS_FILE=$1
if [  ! -f ${RUNS_FILE}.csv ]
then
	echo "File ${RUNS_FILE}.csv doesn't exist"
	exit
fi

echo "#`date`" > ${RUNS_FILE}_fixed.csv
echo "Input file, Type, Depth, FS1, FSN, NumFS, Aggregate type, Num Logical, Num Physical, Num Blocks, Num Containers , Total Bytes, K Bytes, Epsilon Bytes, Moved bytes, Copied Bytes, Moved Files, K, Epsilon, Moved, Copied, Process time [Seconds], Solver time [Seconds], RAM[GB]" >> ${RUNS_FILE}_fixed.csv

while IFS="," read -r -a line
do
	# Disregard comments or header lines
	if [[ ${line[0]} == *[#]* ]] || [[ ${line[0]} == "Input file" ]]
	then
		continue
	fi

	FILENAME=${line[0]}
	echo "Fixing ${FILENAME}"
	FILEPATH=`find . -name ${FILENAME}*`
	if [[ -a ${FILEPATH} ]]; then
		gunzip ${FILEPATH}
		FILEPATH=`find . -name ${FILENAME}*`
	fi

	getMinSysNum
	line[3]=${MINSYS}
	getMaxSysNum
	line[4]=${MAXSYS}
	getNumSystem
	line[5]=${SYSTEM}
	getFiles
	physicalOrLogical
	line[7]=${PHYSICAL}
	line[8]=${LOGICAL}
	getContainers
	line[9]=${CONTAINERS}
	getBlocks
	line[10]=${BLOCKS}

	IFS=","; echo "${line[@]:0:24}" >> ${RUNS_FILE}_fixed.csv

	gzip ${FILEPATH}
done <"${RUNS_FILE}.csv"


