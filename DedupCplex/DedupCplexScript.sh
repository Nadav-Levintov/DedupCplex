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

function log_run {
	TYPE=`cat ${FILE}${CSV} | grep 'Output type:' | cut -d ":" -f 2 | tr -d '[:space:]'`
	if [ "${TYPE}" == "block-level" ]; then
		PHYSICAL="0"
		BLOCKS=`cat ${FILE}${CSV} | grep 'Num blocks' | cut -d ":" -f 2 | tr -d '[:space:]'`
		CONTAINERS="0"
		ATYPE="BLOCK"	
	elif [ "${TYPE}" == "file-level" ]; then
		BLOCKS="0"
		PHYSICAL=`cat ${FILE}${CSV} | grep 'Num physical files' | cut -d ":" -f 2 | tr -d '[:space:]'`	
		CONTAINERS="0"
		ATYPE="FILE"	
	else 
		TYPE="container-level"
		BLOCKS="0"
		PHYSICAL="0"	
		CONTAINERS=`cat ${FILE}${CSV} | grep 'Num of containers' | cut -d ":" -f 2 | tr -d '[:space:]'`
		ATYPE="CONTAINER"
	fi		
	FILES=`cat ${FILE}${CSV} | grep 'Num files' | cut -d ":" -f 2 | tr -d '[:space:]'`
	if [ "${FILES}" == "" ]; then
		FILES=`cat ${FILE}${CSV} | grep 'Num of files' | cut -d ":" -f 2 | tr -d '[:space:]'`
	fi
	IFS='_' read -r -a array <<< "$FILE"
	DEPTH="${array[2]}" 
	if [[ $DEPTH == *"depth"* ]]; then
  		DEPTH=${array[2]#"depth"}
		DEPTH=${DEPTH%"depth"}
		FS1="${array[3]}"
		FSN="${array[4]}"
	else
		DEPTH="0"
		FS1="${array[2]}"
		FSN="${array[3]}"
	fi
	if [ FS1 == "" ]; then
		FS1="0"
	else
		FS1="$(($FS1+0))"
	fi
	if [ FSN == "" ]; then
		FSN="0"
	else
		FSN="$(($FSN+0))"
	fi
	NUMFS="$(($FSN-$FS1))"
	echo -n "${FILE}${CSV},${TYPE},${DEPTH},${FS1},${FSN},${NUMFS},${ATYPE},${FILES}," >> runs.csv
	RESULTFILE=`ls | grep 'result'`
	if [ "${RESULTFILE}" == "" ]; then
		PHYSICAL="0"
		BLOCKS="0"
		CONTAINERS="0"
		TOTAL="0"
		KBYTES="0"
		EPSILONBYTES="0"
		MOVEDBYTES="0"
		COPIEDBYTES="0"
		MOVEDFILES="0"
		MOVEDPERCENT="0"
		COPIEDPERCENT="0"
		INPUTTIME="0"
		SOLVETIME="0"
	else
		TOTAL=`cat ${RESULTFILE} | grep 'Total Size' | cut -d ":" -f 2 | tr -d '[:space:]'`
		KBYTES=`printf "%.2f\n" "$(bc -l <<<  ${TOTAL}*${K}/100 )"`
		EPSILONBYTES=`printf "%.2f\n" "$(bc -l <<<  ${KBYTES}*${EPSILON}/100 )"`
		MOVEDBYTES=`cat ${RESULTFILE} | grep 'Moved Storage' | cut -d ":" -f 2 | tr -d '[:space:]'`
		COPIEDBYTES=`cat ${RESULTFILE} | grep 'Copied Storage' | cut -d ":" -f 2 | tr -d '[:space:]'`
		MOVEDFILES=`cat ${RESULTFILE} | grep 'Moved Files' | cut -d ":" -f 2 | tr -d '[:space:]'`
		MOVEDPERCENT=`printf "%.2f\n" "$(bc -l <<<  ${MOVEDBYTES}/${TOTAL}*100 )"`
		COPIEDPERCENT=`printf "%.2f\n" "$(bc -l <<<  ${COPIEDBYTES}/${TOTAL}*100 )"`
		INPUTTIME=`cat ${RESULTFILE} | grep 'Input time' | cut -d ":" -f 2 | tr -d '[:space:]'`
		SOLVETIME=`cat ${RESULTFILE} | grep 'Solve Time' | cut -d ":" -f 2 | tr -d '[:space:]'`
	fi	
		
	echo -n "${PHYSICAL},${BLOCKS},${TOTAL},${KBYTES},${EPSILONBYTES},${MOVEDBYTES},${COPIEDBYTES},${MOVEDFILES},${K},${EPSILON},${MOVEDPERCENT},${COPIEDPERCENT},${INPUTTIME},${SOLVETIME}" >> runs.csv
	calc_time_and_ram
	rm ${RESULTFILE}
}

function calc_time_and_ram {
	#T=`cat time.csv | cut -d ',' -f 1`
	MEM=`cat time.csv | cut -d ',' -f 2`
	MEM=`printf "%.2f\n" "$(bc -l <<< ${MEM}/${KBTGB} )"`
	echo ",${MEM}" >> runs.csv
}

function run_dedup {
	OUTFILE="${FILE}_K${K}_E${EPSILON}"
    /usr/bin/time -f "%e,%M" -o time.csv ./DedupCplex ${FILE}${CSV} ${K}'%' ${EPSILON}'%' > "${FILE}_K${K}_E${EPSILON}${LOG}" 
	log_run
	rm -rf time.csv
	mv lpex1.lp ${OUTFILE}.lp
}

rm -rf time.csv
if [ -f runs.csv ]
then
	cat runs.csv >> runs_old.csv
	rm -rf runs.csv
fi

echo "#`date`" >> runs.csv
echo "Input file, Type, Depth, FS1, FSN, NumFS, Aggregate type, Num Logical, Num Physical, Num Blocks, Num Containers , Total Bytes, K Bytes, Epsilon Bytes, Moved bytes, Copied Bytes, Moved Files, K, Epsilon, Moved, Copied, Process time [Seconds], Solver time [Seconds], RAM[GB]" >> runs.csv

for FILE in $( ls -Sr | grep ".csv" | cut -f 1 -d . | grep -v "runs" ); do
    rm -rf ${FILE}
	rm -rf time.csv
	mkdir ${FILE}
	for K in 10 20 30 40 50; do
		for EPSILON in 0.5 1 2 5; do
			run_dedup
		done
	done
    mv *${FILE}*.* ${FILE}
done
