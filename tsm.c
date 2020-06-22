/*
Author: Jiaqi Liu
Traveling Salesman Problem
Parallel Computing
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

typedef struct{
    int visitCount;
    int cost;
    int curCity;
    int* pre;
    unsigned char* visited;
} Path;


typedef struct{
    Path** data;
    int logicalSize;
    int physicalSize;
} Stack;


/*
This function calculates the factorial of n.
*/
int factorial(int n){
    int res = 1;
    int i;
    for(i=1;i<=n;res*=i++);
    return res;
}

/*
The function initializes a new Stack struct and set the input Stack pointer
pointing to the Stack.
It deals with memory allocations and it is responsible for setting the
initial field values of Stack struct.
*/
void initStack(Stack** S,int N){
    *S = (Stack*)malloc(sizeof(Stack) * 1);
    if(*S!=NULL){
        int capacity = factorial(N);
        (*S)->logicalSize = 0;
        (*S)->physicalSize = capacity;
        (*S)->data = (Path**)malloc(sizeof(Path*) * capacity);
    }
}

/*
The function initializes a new Path struct and set the input Path pointer
pointing to the new struct.

It contains two modes:
1. initialize a new path based on an old path, where all the new path fields
are identical to the old path.
2. initialize a new path.

If the old path input is not NULL, then it will enter mode 1, otherwise mode 2.

It deals with memory allocations and it is responsible for setting the
initial field values of the Path struct.
*/
void initPath(Path* oldPath,int N,Path** res){
    unsigned char* visited = (unsigned char*)malloc(sizeof(unsigned char) * N);
    int* pre = (int*)malloc(sizeof(int) * N);
    if(visited == NULL || pre == NULL){
        printf("Error allocating memory\n");
        exit(1);
    }
    *res = (Path*)malloc(sizeof(Path));
    // Mode 2: initialize a new path.
    if(oldPath == NULL){
        memset(visited,0,sizeof(unsigned char)*N);
        memset(pre,-1,sizeof(int)*N);
        (*res)->cost = 0;
        (*res)->visitCount = 0;
        (*res)->curCity = 0;
        (*res)->visited = visited;
        (*res)->pre = pre;
    }
    // Mode 1: initialize a new path based on an old path,
    // where all the new path fields are identical to the old path.
    else{
        (*res)->visitCount = oldPath->visitCount;
        memcpy(visited,oldPath->visited,sizeof(unsigned char) * N);
        memcpy(pre,oldPath->pre, sizeof(int) * N);
        (*res)->visited = visited;
        (*res)->pre = pre;
        (*res)->cost = oldPath->cost;
        (*res)->curCity = oldPath->curCity;
    }
}

/*
The function allows pushing a path struct on a stack.
*/
void push(Stack* S, Path* p){
    if(S->logicalSize > S->physicalSize){
        printf("Stack Overflow\n");
        exit(1);
    }
    S->data[S->logicalSize] = p;
    S->logicalSize = S->logicalSize + 1;
}

/*
The function allows poping a path struct off a stack.
After the function is complete, the Path pointer pointed by pointer
p will be pointing to the poped path struct.
*/
void pop(Stack* S, Path** p){
    *p = NULL;
    if(S->logicalSize > 0){
        *p = S->data[S->logicalSize-1];
        S->data[S->logicalSize-1] = NULL;
        S->logicalSize -= 1;
    }
}

/*
The function performs a parallelized depth-first search to find the
optimal solution.
After the function is complete, the Path pointer pointed by pointer
best will be pointing to the optimal Path struct.
*/
void dfs(int** matrix,int N,Path** bestPath,Path** workPoll,int thread_count) {
    int cost = INT_MAX;
    #  pragma omp parallel num_threads(thread_count)
    {
      int num_threads = omp_get_num_threads();
      int my_rank = omp_get_thread_num();
      // divide work among threads based on their rank
      int local_n = N / num_threads;
      int local_begin;
      int local_end;
      local_begin = local_n*my_rank;
      local_end = local_begin + local_n;
      // The last thread will handle a larger workload if the problem size
      // cannot equally divede among threads.
      if(my_rank == num_threads-1) local_end = N-1;
      int j;
      int i;
      Stack* S;
      initStack(&S,N);
      // Each thread extracts work from work poll and push onto the Stack
      // for search.
      for(j=local_begin;j<local_end;++j){
          Path* p = workPoll[j];
          push(S,p);
      }
      // perform depth-first search
      while(S->logicalSize > 0){
          Path* oldPath;
          pop(S,&oldPath);
          // better path detected
          if(oldPath->visitCount == N && oldPath->cost < cost){
            // update optimal path
            #pragma omp critical
            {
              *bestPath = oldPath;
              cost = oldPath->cost;
            }
          }
          else if(oldPath->cost < cost){
              for(i=N-1;i>0;--i){
                  int newCost = oldPath->cost + matrix[oldPath->curCity][i];
                  // Pruning: before reaching the goal state, an optimal path
                  // must not equal or larger than the optimal cost so far.
                  if(oldPath->visited[i] == 0 && newCost < cost){
                    // create a successor state based on the current state
                      Path* newPath;
                      initPath(oldPath,N,&newPath);
                      newPath->cost = newCost;
                      newPath->visited[i] = 0b1;
                      newPath->pre[newPath->visitCount] = i;
                      newPath->visitCount += 1;
                      newPath->curCity = i;
                      // push the sucessor state to Stack and keep searching
                      push(S,newPath);
                  }
              }
          }
      }
      free(S->data);
      free(S);
    }
}

/*
The function outputs the solution path and the distance to the standard output.
*/
void printSolution(Path* best,int N){
    int i;
    printf("Best path:");
    for(i=0;i<N;++i) printf(" %d",(*best).pre[i]);
    printf("\n");
    printf("Distance: %d\n",best->cost);
}
/*
The function divides the search task into to N-1 tasks, and inserts all the
divided tasks into work poll.

The work will be divided by threads during search stage.

After the function is complete, the Path pointer pointed by pointer workpoll
will be pointing to an array of N-1 sub paths begining from city 0.
*/
void divideWork(int N,int** matrix,Path** workPoll){
    int i;
    Path* p;
    initPath(NULL,N,&p);
    p->visited[0] = 0;
    p->visitCount = 1;
    p->pre[0] = 0;
    // each sub path represents a way to leave city 0
    for(i = N-1; i > 0; --i){
        Path* subPath;
        initPath(p,N,&subPath);
        subPath->cost =  matrix[0][i];
        subPath->visited[i] = 0b1;
        subPath->pre[subPath->visitCount] = i;
        subPath->visitCount += 1;
        subPath->curCity = i;
        // add the subpath to workpoll
        workPoll[i-1] = subPath;
    }
}

int main(int argc, char* argv[]) {
    // checking program arguments
    if(argc != 4){
        printf("Usage: ./tsm num N filename.txt\n");
        exit(1);
    }
    // process inputs - converting them into related data structures
    int i,j,N,num;
    sscanf(argv[1],"%d",&num);
    sscanf(argv[2],"%d",&N);
    const char* filename = argv[3];
    FILE* fp;
    fp = fopen(filename,"r");
    int** matrix = (int**)malloc(sizeof(int*) * N);
    for(i=0;i<N;++i){
        matrix[i] = (int*)malloc(sizeof(int) * N);
        for(j=0;j<N;++j){
            fscanf(fp, "%d", &matrix[i][j]);
        }
    }
    fclose(fp);
    // prepare a work poll to be diveded by threads
    Path** workPoll = (Path**)malloc(sizeof(Path*)*(N-1));
    divideWork(N,matrix,workPoll);
    // start searching for solutions in parallel
    Path* best;
    dfs(matrix,N,&best,workPoll,num);
    // output the optimal solution to the standard output
    printSolution(best,N);
    // release allocated memory
    free(matrix);
    free(workPoll);
    return 0;
}
