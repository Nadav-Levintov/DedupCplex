#!/bin/bash
CSV=".csv"
LOG=".log"
SIZE="4"
KB="1024"
MB=$(( KB * KB ))
TOTALS=$(( SIZE * MB ))
DISTANCE="0"
POINTERS="0"
OUTFILE=""
KBTGB=$MB

function log_run {
	echo -n "${FILE}${CSV},${TOTALS},${POINTERS}," >> runs.csv
	calc_time_and_ram
	FILES=`cat ${FILE}${CSV} | grep 'Num files' | cut -d ":" -f 2 | tr -d '[:space:]'`
	BLOCKS=`cat ${FILE}${CSV} | grep -e 'Num blocks' -e 'Num physical files' | cut -d ":" -f 2 | tr -d '[:space:]'`	
	CONTAINERS=`cat ${OUTFILE}${CSV} | grep 'Num of containers' | cut -d ":" -f 2 | tr -d '[:space:]'`
	echo ",${FILES},${BLOCKS},${CONTAINERS}" >> runs.csv
}

function calc_time_and_ram {
	T=`cat time.csv | cut -d ',' -f 1`
	MEM=`cat time.csv | cut -d ',' -f 2`
	MEM=`printf "%.2f\n" "$(bc -l <<< ${MEM}/${KBTGB} )"`
	echo -n "${T},${MEM}" >> runs.csv
}

function run_dedup {
	OUTFILE="${FILE}_${TOTALS}_D${DISTANCE}_P${POINTERS}"
    /usr/bin/time -f "%e,%M" -o time.csv ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${TOTALS}_D${DISTANCE}_P${POINTERS}${LOG}" 
	log_run
	rm -rf time.csv
}

rm -rf time.csv
if [ -f runs.csv ]
then
	cat runs.csv >> runs_old.csv
	rm -rf runs.csv
fi

echo "Input file, Max Container size, Max Pointers to block, Run time [Seconds], RAM[GB], Files in input, Blocks in input, Containers in output" >> runs.csv
echo "#`date`" >> runs.csv


for FILE in $( ls -Sr | grep ".csv" | cut -f 1 -d . | grep -v "runs" ); do
    rm -rf ${FILE}
	rm -rf time.csv
	mkdir ${FILE}

	#pointers=0, size=4MB
	SIZE="4"
    TOTALS=$((SIZE*MB))
    POINTERS="0"
	run_dedup

	#pointers=128, size=4MB
	POINTERS="128"
	run_dedup

	#pointers=0, size=8MB
    SIZE="8"
    TOTALS=$((SIZE*MB))
    POINTERS="0"
	run_dedup

	#pointers=128, size=8MB
    POINTERS="128"
	run_dedup

    mv *${FILE}*.* ${FILE}

done
