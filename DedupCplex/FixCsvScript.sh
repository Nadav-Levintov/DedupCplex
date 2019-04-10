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
FILETYPE=""

function getMinSysNum {
	MINSYS=`cat ${FILEPATH} | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort -n | head -1`
}

function getMaxSysNum {
	MAXSYS=`cat ${FILEPATH} | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | sort -n | tail -1`
}

function getContainers {
	CONTAINERS=`cat ${FILEPATH} | grep "#" | grep "containers" -i | grep "num" -i | cut -f2 -d":"`
	if [ -z "${CONTAINERS}" ]
	then
	      CONTAINERS="0"
	fi
}

function getBlocks {
	BLOCKS=`cat ${FILEPATH} | grep "#" | grep "blocks" -i | grep "num" -i | cut -f2 -d":"`

	if [ -z "${BLOCKS}" ]
	then
	      BLOCKS="0"
	fi
}

function getFiles {
	FILES=`cat ${FILEPATH} | grep "#" | grep "files" -i | grep "num" -i | cut -f2 -d":"`
}

function getNumSystem {
	SYSTEM=`cat ${FILEPATH}  | grep "F" | cut -f3 -d"," | cut -f1 -d"_" | uniq | wc -l`
}

function physicalOrLogicalMP {
	LOGICAL=`cat ${FILEPATH} | grep "#" | grep "num files" -i | cut -f2 -d":"`

	if [[ ${FILEPATH} =~ .*_b_.* ]] || [[ ${FILEPATH} =~ .*_B_.* ]] || [[ ${FILEPATH} =~ .*B_.* ]]
	then
		FILETYPE="block-level"
		PHYSICAL="0"
	else
		FILETYPE="file-level"
		PHYSICAL=`cat ${FILEPATH} | grep "#" | grep "files" -i | grep "num" -i | grep "physical" -i | cut -f2 -d":"`
		BLOCKS="0"
	fi
}

function physicalOrLogical {
	LOGICAL=${FILES}

	if [[ ${FILEPATH} =~ .*_b_.* ]] || [[ ${FILEPATH} =~ .*_B_.* ]] || [[ ${FILEPATH} =~ .*B_.* ]]
	then
		FILETYPE="block-level"
		PHYSICAL="0"
	else
		FILETYPE="file-level"
		PHYSICAL=${BLOCKS}
		BLOCKS="0"
	fi
}

function getSystemMP {
	MINSYS=`echo "${FILENAME}"  | cut -d"." -f1 | cut -d"_" -f4`
	MINSYS=${MINSYS##+(0)}
	MAXSYS=`echo " ${FILENAME}"  | cut -d"." -f1 | cut -d"_" -f5`
	MAXSYS=${MAXSYS##+(0)}
	SYSTEM=$(( ${MAXSYS} - ${MINSYS} + 1 ))
}

RUNS_FILE=$1
if [  ! -f ${RUNS_FILE}.csv ]
then
	echo "File ${RUNS_FILE}.csv doesn't exist"
	exit
fi

echo "#`date`" > ${RUNS_FILE}_fixed.csv
echo "Input file, Type, Depth, FS1, FSN, NumFS, Aggregate type, Num Logical, Num Physical, Num Blocks, Num Containers , Total KBs, K KBs, Epsilon KBs, Moved KBs, Copied KBs, Moved Files, K, Epsilon, Moved, Copied, Process time [Seconds], Solver time [Seconds], RAM[GB]" >> ${RUNS_FILE}_fixed.csv

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
	if [[ -a ${FILEPATH} ]] &&  [[ ${FILEPATH} == *gz* ]]
	then
		gunzip ${FILEPATH}
		FILEPATH=`find . -name ${FILENAME}*`
	fi
	
	if [[ ! -a ${FILEPATH} ]]
	then
		continue
	fi

	#Fix need_to_run_cplex
	getSystemMP
	line[3]=${MINSYS}
	line[4]=${MAXSYS}
	line[5]=${SYSTEM}
	getBlocks
	getFiles
	physicalOrLogicalMP
	getContainers
	line[1]=${FILETYPE}
	line[7]=${LOGICAL}
	line[8]=${PHYSICAL}
	line[9]=${BLOCKS}
	line[10]=${CONTAINERS}

	IFS=','; echo "${line[*]}" >> ${RUNS_FILE}_fixed.csv

	#gzip ${FILEPATH}
done <"${RUNS_FILE}.csv"

echo "All Done"
