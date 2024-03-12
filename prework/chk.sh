#!/usr/bin/bash
# chk.sh 
# chmod +x ./chk.sh

echo "compiling"
g++ make.cpp -o mk.out -Wall -fsanitize=undefined -O2
g++ BPlusTree.cpp -o tst.out -Wall -fsanitize=undefined -O2
g++ database.cpp -o std.out -Wall -fsanitize=undefined -O2
echo "compiled"

for ((i=1;;i++))
do
    ./mk.out > in.txt
    echo "running on test" $i
    rm head.db data.db data_file.db
    ./std.out < in.txt > ans.txt
    ./tst.out < in.txt > out.txt
    if [ $? -ne 0 ] ; then 
        echo "Runtime Error"
        break
    fi
    diff ans.txt out.txt -Z
    if [ $? -ne 0 ] ; then
        echo "Wrong Answer"
        break
    fi
done