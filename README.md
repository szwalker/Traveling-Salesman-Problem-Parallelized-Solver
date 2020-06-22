# Parallelized Solver for Traveling Salesman Problem

## Problem Definition
Given a group of n cities (from 0 to n-1). The traveling salesman must start from city number 0, visit each city once, and does not have to come back to the initial city. The distance between each city (cities are fully connected to each other) will be provided in a matrix format. The program will pick the path with shortest distance. When there exists be several paths that satisfy the above conditions, the program will only find and return one of them.

## Potential Challenges
This problem is a combinatorial optimization problem (n! possible solutions for n cities). One of the straightforward ways to solve it is through branch and bound (imagine constructing a tree that describes all the possible routes from the first city). However, the tree may get too large for large problems and it takes a long time for a sequential program to compute the solution.

## Parallelized Solver
Please see my source code in `tsm.c`.

## Program Input
The input to your program is a text file that contains an NxN matrix, one row per line, representing the distance between any two cities. Your program will be called tsm. The command line then will be:

```
./tsm num N filename.txt
```
Where:
* `num` is the number of threads
* `N` is the number of cities
* `filename.txt` is the file that contains the distance matrix among the N cities

Here is an example of input file with 11 cities.
```
0 1 1 1 1 1 1 1 1 1 1
1 0 2 18 12 8 16 15 10 12 17
1 2 0 2 2 2 2 2 2 2 2
1 18 2 0 3 12 8 12 7 9 8
1 12 2 3 0 2 3 3 3 3 3
1 8 2 12 2 0 12 4 5 4 6
1 16 2 8 3 12 0 10 3 9 1
1 15 2 12 3 4 10 0 3 13 1
1 10 2 7 3 5 3 3 0 3 3
1 12 2 9 3 4 9 13 3 0 1
1 17 2 8 3 6 1 1 3 1 0
```


## Program Output
The program will output, to standard output stream, the best path you found and the total distance. Here is an example output:
```
Best path: 0 1 3 4 5 2
Distance: 36
```
This means the best path your program found is 0->1->3->4->5->2 with total distance of 36.


## Benchmark
Here is a problem generator and an near optimal sequential solver for speed comparison.

To use them, you have to do the one-time setup using the following commands.
```
chmod 777 ./tsmoptimal
chmod 777 ./gentsm
``` 

Problem generator will generate a text file that matches the problem input description. You can use your own input or use `./gentsm num` where `num` is the number of cities.

To execute the optimal sequential solver, use `./tsmoptimal num filename` where `num` is the number of cities and `filename` is the file that contains the distances among cities. (Please note that this sequential solver is not able to handle input size that is larger than 12).

## Result
The parallelized solver can achieve 1.5 speedup compared with the sequential benchmark and can expect much larger speedup with larger input sizes. Unfortunatelly the benchmark is not able to handle larger problem sizes.

