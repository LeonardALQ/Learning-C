#!/usr/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
STOP='\033[0m'

###################### COMPLEX ##########################
FILE="Testcases/Complex/complex"
LENGTH=3

echo "COMPLEX TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### ADD ##########################
FILE="Testcases/Add/add"
LENGTH=4

echo "ADD TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### DATA ##########################
FILE="Testcases/Data/data"
LENGTH=3

echo "DATA TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### REFERENCES ##########################
FILE="Testcases/References/reference"
LENGTH=5

echo "REFERENCES TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### REMOVE ##########################
FILE="Testcases/Remove/remove"
LENGTH=3

echo "REMOVE TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### SIMPLE ##########################
FILE="Testcases/Simple/simple"
LENGTH=3

echo "SIMPLE TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done
echo " "

####################### SNAPSHOT ##########################
FILE="Testcases/Snapshot/snapshot"
LENGTH=5

echo "SNAPSHOT TESTCASES:"
for ((i = 1; i <= LENGTH; i++))
do 
    if ./ymirdb < "$FILE$LENGTH".in | diff - "$FILE$LENGTH".out &> /dev/null; then 
        echo -e "  TESTCASE: $i ${GREEN}PASSED${STOP}"
    else 
        echo -e "  TESTCASE $i: ${RED}FAILED${STOP}"
    fi
done