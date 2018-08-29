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
	RESULTFILE=`ls | grep 'result'`
	#cat ${RESULTFILE}
	TYPE=`cat ${FILE}${CSV} | grep 'Output type:' | cut -d ":" -f 2 | tr -d '[:space:]'`
	FILES=`cat ${FILE}${CSV} | grep 'Num files' | cut -d ":" -f 2 | tr -d '[:space:]'`
	echo -n "${FILE}${CSV},${TYPE},${FILES}," >> runs.csv
	if [ "${TYPE}" == "block-level" ]; then
		PHYSICAL="0"
		BLOCKS=`cat ${FILE}${CSV} | grep 'Num blocks' | cut -d ":" -f 2 | tr -d '[:space:]'`	
	else
		BLOCKS="0"
		PHYSICAL=`cat ${FILE}${CSV} | grep 'Num physical files' | cut -d ":" -f 2 | tr -d '[:space:]'`	
	fi
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
}

rm -rf time.csv
if [ -f runs.csv ]
then
	cat runs.csv >> runs_old.csv
	rm -rf runs.csv
fi

echo "#`date`" >> runs.csv
echo "Input file, Type,  Num Logical, Num Physical, Num Blocks, Total Bytes, K Bytes, Epsilon Bytes, Moved bytes, Copied Bytes, Moved Files, K, Epsilon, Moved, Copied, Process time [Seconds], Solver time [Seconds], RAM[GB]" >> runs.csv

for FILE in $( ls -Sr | grep ".csv" | cut -f 1 -d . | grep -v "runs" ); do
    rm -rf ${FILE}
	rm -rf time.csv
	mkdir ${FILE}

	for K in 10 20 30; do
		for EPSILON in 0.01 0.1 0.5; do
			run_dedup
		done
	done
    mv *${FILE}*.* ${FILE}
done
