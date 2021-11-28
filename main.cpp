//To be completed:
// Lecturer: Dr. Benjamin Mora
// Student name: Yusuf Dauda
// date: 18/11/2021

/**Most of the code defining the problem is written by the Lecturer Dr.Ben Mora
* Mutex, all lock implementations and the methods that implement the locks, the function
* (WalkersI) and all associated functions, variables, vectors, maps, grids, and threads created
* are all done by me the student.
*/

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <map>

#define N 1000
#define S 25
#define MAX_WALKERS_PER_LOCATION 3
#define MAX_WALKERS_PER_EDGE 4



typedef struct Walker
{
    int currentX, currentY, finalX, finalY;
    bool hasArrived;
    void Init() 
    {
        currentX = rand() % S;
        currentY = rand() % S;
        finalX = rand() % S;
        finalY = rand() % S;
        hasArrived = false;
    }
};

int originalGridCount[S][S];  //used to make sure the number of walkers per location is within the limits. Do not modify!
int finalGridCount[S][S];  //used to set the result target.  Do not modify!
int obtainedGridCount[S][S];  //something you may use for the results
Walker walkers[N];
std::mutex m; 


void Lock(std::mutex* m)
{
    m->lock();
    std::this_thread::sleep_for(std::chrono::microseconds(1)); //Sleeps to simulate longer execution time and increase the probability of an issue
}

void Unlock(std::mutex* m)
{
    m->unlock();
}

void PrintGrid(std::string message, int grid[S][S])
{
    std::cout << message;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
            std::cout << (grid[i][j]<10?"  " : " ") << grid[i][j]; //Initialising grids
        std::cout << "\n";
    }
}

void SetObtainedGrid()
{
    for (int i = 0; i < N; i++) //Initialising walkers' locations
    {
        obtainedGridCount[walkers[i].currentY][walkers[i].currentX]++;
        if (!walkers[i].hasArrived)
        {
            std::cout << "\nAt least one walker had not arrived!\n";
            return;
        }
    }
}

void CompareGrids(int a[S][S], int b[S][S])
{
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            if (a[i][j] != b[i][j])
            {
                std::cout<<"\nError: results are different!\n";
                return;
            }
    std::cout << "\nSeems to be OK!\n";
}


// ####################   Write Most of your code here ################################################################
int V = S * S; //used to initialize visited vector
std::vector<bool> visited;
int edge[S*S][S*S];
std::map <int, std::vector<bool>> threadVisitLog;


int Position(int y, int x) 
{
    Lock(&m);
    int position = (y * S) + x; //position of vertex on Grid
    Unlock(&m);
    return position;
}


void EnterVertex(int y, int x)
{
    Lock(&m);
    obtainedGridCount[y][x]++; 
    Unlock(&m);
}

void EnterEdge(int src, int dest)
{
    Lock(&m);
    edge[src][dest]++; //src is starting position, dest is next position
    Unlock(&m);
}

void ExitVertex(int y, int x)
{
    Lock(&m);
    obtainedGridCount[y][x]--;
    Unlock(&m);
}

void ExitEdge(int src, int dest)
{
    Lock(&m);
    edge[src][dest]--; //edge connecting src and destination
    Unlock(&m);
}

bool IsAvailabe(int y, int x) 
{
    Lock(&m);
    bool available = obtainedGridCount[y][x] < MAX_WALKERS_PER_LOCATION;
    Unlock(&m);
    return available;
}

bool IsEdgeFree(int src, int dest) 
{
    Lock(&m);
    bool free = (edge[src][dest] + edge[dest][src]) < MAX_WALKERS_PER_EDGE;
    Unlock(&m);
    return free;
        
}

bool IsVisited(int y, int x, int id) 
{
    int position = Position(y,x);
    Lock(&m);
    bool visitor = threadVisitLog.at(id).at(position) == true;
    Unlock(&m);
    return visitor; 
}

void Move(int ysrc, int xsrc, int ydest, int xdest, int src, int dest) // steps to move from src to dest
{
    ExitVertex(ysrc, xsrc);
    EnterEdge(src, dest);
    ExitEdge(src, dest);
    EnterVertex(ydest, xdest);
   
}

void MoveLocation(int *y, int *x, int *id, int *order)  //first check all non visited locations
{
    if (*y < S - 1 && !IsVisited(*y + 1, *x, *id) && IsAvailabe(*y + 1, *x)
        && IsEdgeFree(Position(*y, *x), Position(*y + 1, *x)))
    {
        Move(*y, *x, *y + 1, *x, Position(*y, *x), Position(*y + 1, *x));
        threadVisitLog.at(*id).at(Position(*y + 1, *x)) = 1;
        *y = *y + 1;
    }
    else if (*x > 0 && !IsVisited(*y, *x - 1, *id) && IsAvailabe(*y, *x - 1)
        && IsEdgeFree(Position(*y, *x), Position(*y, *x - 1)))
    {
        Move(*y, *x, *y, *x - 1, Position(*y, *x), Position(*y, *x - 1));
        threadVisitLog.at(*id).at(Position(*y, *x - 1)) = 1;
        *x = *x - 1;
    }
    else if (*y > 0 && !IsVisited(*y - 1, *x, *id) && IsAvailabe(*y - 1, *x)
        && IsEdgeFree(Position(*y, *x), Position(*y - 1, *x)))
    {
        Move(*y, *x, *y - 1, *x, Position(*y, *x), Position(*y - 1, *x));
        threadVisitLog.at(*id).at(Position(*y - 1, *x)) = 1;
        *y = *y - 1;
    }
    else if (*x < S - 1 && !IsVisited(*y, *x + 1, *id) && IsAvailabe(*y, *x + 1)
        && IsEdgeFree(Position(*y, *x), Position(*y, *x + 1)))
    {
        Move(*y, *x, *y, *x + 1, Position(*y, *x), Position(*y, *x + 1));
        threadVisitLog.at(*id).at(Position(*y, *x + 1)) = 1;
        *x = *x + 1;
    }

    else if (*order == 1)  // randomly check visited locations
    {
        if (*y < S - 1 && IsAvailabe(*y + 1, *x) && IsEdgeFree(Position(*y, *x), Position(*y + 1, *x)))
        {
            Move(*y, *x, *y + 1, *x, Position(*y, *x), Position(*y + 1, *x));
            *y = *y + 1;
        }
        else if (*x > 0 && IsAvailabe(*y, *x - 1) && IsEdgeFree(Position(*y, *x), Position(*y, *x - 1)))
        {
            Move(*y, *x, *y, *x - 1, Position(*y, *x), Position(*y, *x - 1));
            *x = *x - 1;
        }
    }
    else 
    {
        if (*y > 0 && IsAvailabe(*y - 1, *x) && IsEdgeFree(Position(*y, *x), Position(*y - 1, *x)))
        {
            Move(*y, *x, *y - 1, *x, Position(*y, *x), Position(*y - 1, *x));
            *y = *y - 1;
        }
        else if (*x < S - 1 && IsAvailabe(*y, *x + 1) && IsEdgeFree(Position(*y, *x), Position(*y, *x + 1)))
        {
            Move(*y, *x, *y, *x + 1, Position(*y, *x), Position(*y, *x + 1));
            *x = *x + 1;
        }
    }
}

void WalkerI(int id)
{
    //Write your thread here
    EnterVertex(walkers[id].currentY, walkers[id].currentX);
    threadVisitLog.at(id).at(Position(walkers[id].currentY, walkers[id].currentX)) = true;

    while (!walkers[id].hasArrived) 
    {
        int order = rand() % 2;
        MoveLocation(&walkers[id].currentY, &walkers[id].currentX, &id, &order);
        if (walkers[id].currentY == walkers[id].finalY
            && walkers[id].currentX == walkers[id].finalX) 
        {
            walkers[id].hasArrived = true;
            ExitVertex(walkers[id].currentY, walkers[id].currentX);
            threadVisitLog.at(id).clear();
        }
    }
  
}

// ####################   End of your code ################################################################


void InitGame()
{
   
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            originalGridCount[i][j] = finalGridCount[i][j] = obtainedGridCount[i][j] = 0; //Initialising grids
    for (int i = 0; i < N; i++) //Initialising walkers' locations
    {
        do walkers[i].Init();
        while (originalGridCount[walkers[i].currentY][walkers[i].currentX]>= MAX_WALKERS_PER_LOCATION);
        originalGridCount[walkers[i].currentY][walkers[i].currentX]++;
    }
    for (int i = 0; i < N; i++) //Initialising walkers' locations
        finalGridCount[walkers[i].finalY][walkers[i].finalX]++;

    for (int i = 0; i < V; i++)
    {
        visited.push_back(false); //initialize all visited to false
    }
    for (int i = 0; i < N; i++)
    {
        threadVisitLog.insert({ i, visited }); //initialize map to track individual thread visits
    }
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++)
            edge[i][j] = 0; //initializing edges 
}

int main()
{
    InitGame();
    //Start your threads here.
    std::thread t[N];
    for (int i = 0; i < N; i++)
    {
        t[i] = std::thread(WalkerI, i);
    }
    for (int i = 0; i < N; i++) 
    {
        t[i].join();
    }
    PrintGrid("Original locations:\n\n", originalGridCount);
    PrintGrid("\n\nIntended Result:\n\n", finalGridCount);
    SetObtainedGrid();
    PrintGrid("\n\nObtained Result:\n\n", obtainedGridCount);
    CompareGrids(finalGridCount, obtainedGridCount);
}
