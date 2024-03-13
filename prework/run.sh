rm data_file.db
g++ BPlusTree.cpp -o test.out -Wall -Wextra -std=c++20 -fsanitize=address -fsanitize=undefined && time -p ./test.out < in.txt > output.txt