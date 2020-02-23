#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <vector>
#include <chrono>
#include <queue>
#include <map>
#include <time.h>
using namespace std;

// for BONUS
string consecutiveDirection = "none";
int numConsecutive = 0;
int maxConsecutive = 1;
bool isConsecutive = false;

class Car;
class Intersection;
vector<string> directionsInIntersection;
bool isInVector(vector<string> directions, vector<string> conflictingDirections);
bool isConflict(Car* car, vector<string> potentialDirectionsInIntersection);
void setOriginalLocation(Car* car);
bool carShouldWait(Car* car, vector<string> directions);
bool isCarInIntersection(Car* car);
void setOriginalLocation(Car* car);
queue<Car*> NorthernCars, SouthernCars, EasternCars, WesternCars;
vector<Car*> carsInIntersection;
vector<Car*> cars;
vector<queue<Car*>*> directionQueues = {&NorthernCars, &SouthernCars, &WesternCars, &EasternCars};
mutex m_mutex;
int numDone = 0;
condition_variable northCarGo;
condition_variable southCarGo;
condition_variable eastCarGo;
condition_variable westCarGo;
string finalOutput = "";
map<string, int> carIDs = {
    {"N", 0},
    {"NW", 0},
    {"NE", 0},
    {"E", 0},
    {"EN", 0},
    {"ES", 0},
    {"W", 0},
    {"WN", 0},
    {"WS", 0},
    {"S", 0},
    {"SW", 0},
    {"SE", 0}
};

class Car
{
public:
    int id;
    int arrivalTime;
    string direction;
    string originDirection;
    queue<Car*>* carQueue;
    condition_variable* cv;
    bool waited = false;
    bool consecutiveWait = false;
    Car(int arrivalTime, string direction)
    {
        this->arrivalTime = arrivalTime;
        this->direction = direction;
    }
};
void setOriginalLocation(Car* car)
{
    switch(car->direction[0])
    {
        case('N'):
            SouthernCars.push(car);
            car->carQueue = &SouthernCars;
            car->cv = &southCarGo;
            car->originDirection = "S";
            break;
        case('E'):
            WesternCars.push(car);
            car->carQueue = &WesternCars;
            car->cv = &westCarGo;
            car->originDirection = "W";
            break;
        case('S'):
            NorthernCars.push(car);
            car->carQueue = &NorthernCars;
            car->cv = &northCarGo;
            car->originDirection = "N";
            break;
        case('W'):
            EasternCars.push(car);
            car->carQueue = &EasternCars;
            car->cv = &eastCarGo;
            car->originDirection = "E";
            break;
    }
}
class Intersection
{
public:
    void tryTravel(Car* car)
    {
        startTravel(car);
        travel(car);
        doneTravel(car);
    }
    void startTravel(Car* car)
    {
        unique_lock<mutex> lock(m_mutex);
        setOriginalLocation(car);
        car->id = carIDs[car->direction]++;
        
        while(carShouldWait(car, directionsInIntersection))
        {
            car->waited = true;
            car->cv->wait(lock);
        }
        
        if(!isCarInIntersection(car))
        {
            carsInIntersection.push_back(car);
            directionsInIntersection.push_back(car->direction);
            car->carQueue->pop();

            if(isConsecutive)
            {
                if((consecutiveDirection != car->originDirection) || (numConsecutive == maxConsecutive))
                {
                    consecutiveDirection = car->originDirection;
                    numConsecutive = 0;
                    car->consecutiveWait = false;
                }
                numConsecutive++;
            }

        }
        if(isConsecutive && car->consecutiveWait)
        {
            this_thread::sleep_for(chrono::seconds(1));
        }
        lock.unlock();
    }
    void travel(Car* car)
    {
        string output = car->direction + " #" + to_string(car->id) + " arrived at intersection\n";
        cout << output;
        this_thread::sleep_for(chrono::seconds(5));
    }
    void doneTravel(Car* car)
    {
        unique_lock<mutex> lock(m_mutex);

        numDone++;
        finalOutput += to_string(car->arrivalTime) + " " + car->direction + "\n";
        string output = car->direction + " #" + to_string(car->id) + " left intersection\n";
        cout << output;
        
        directionsInIntersection.erase(find(directionsInIntersection.begin(), directionsInIntersection.end(), car->direction));
        carsInIntersection.erase(find(carsInIntersection.begin(), carsInIntersection.end(), car));
        
        lock.unlock();
    }
};
bool isCarInIntersection(Car* car)
{
    return find(carsInIntersection.begin(), carsInIntersection.end(), car) != carsInIntersection.end();
}
bool isHead(Car* car)
{
    switch(car->direction[0])
    {
        case('N'): return SouthernCars.size() > 0 && car == SouthernCars.front();
        case('E'): return WesternCars.size() > 0 && car == WesternCars.front();
        case('S'): return NorthernCars.size() > 0 && car == NorthernCars.front();
        case('W'): return EasternCars.size() > 0 && car == EasternCars.front();
    }
    return false;
}
bool carShouldWait(Car* car, vector<string> directions)
{
    if(directions.size() == 0)
        return false;
    if(!isHead(car))
        return true;
    if(isConsecutive)
    {
        if(consecutiveDirection == car->originDirection && numConsecutive < maxConsecutive)
        {
            car->consecutiveWait = true;
            return false;
        }
    }
    for(int i = 0; i < directions.size(); i++)
    {
        if((car->direction)[0] == (directions[i])[0])
            return true;
    }
    
    vector<string> conflictingDirections;
    if(car->direction == "N")
        conflictingDirections = {"E","EN","W","WN","WS","SE"};
    else if(car->direction == "E")
        conflictingDirections = {"N","S","NE","NW","SE","WS"};
    else if(car->direction == "S")
        conflictingDirections = {"NW","E","EN","ES","W","WS"};
    else if(car->direction == "W")
        conflictingDirections = {"N","S","NW","SE","SW","EN"};
    else if(car->direction == "NE")
        conflictingDirections = {"E","SE"};
    else if(car->direction == "NW")
        conflictingDirections = {"S","E","W","SW","EN","WS"};
    else if(car->direction == "SE")
        conflictingDirections = {"N","E","W","NE","EN","WS"};
    else if(car->direction == "SW")
        conflictingDirections = {"W","NW"};
    else if(car->direction == "EN")
        conflictingDirections = {"N","S","W","NW","SE","WN"};
    else if(car->direction == "ES")
        conflictingDirections = {"S","WS"};
    else if(car->direction == "WN")
        conflictingDirections = {"N","EN"};
    else if(car->direction == "WS")
        conflictingDirections = {"N","S","E","NW","SE","ES"};
    
    return isInVector(directions, conflictingDirections);
}
// check if any conflicting cars exist in intersection
bool isInVector(vector<string> directions, vector<string> conflictingDirections)
{
    for(int i = 0; i < conflictingDirections.size(); i++)
    {
        if(find(directions.begin(), directions.end(), conflictingDirections[i]) != directions.end())
            return true;
    }
   return false;
}
void signalingThread()
{
    bool isSignaled;
    vector<string> directionsCopy;
    string output;
    while(true)
    {
        unique_lock<mutex> lock(m_mutex);

        directionsCopy = directionsInIntersection;

        if(numDone < cars.size())
        {
            isSignaled = false;
            for(int i = 0; i < directionQueues.size(); i++)
            {
                if(!directionQueues[i]->empty() && !carShouldWait(directionQueues[i]->front(), directionsCopy) && !isCarInIntersection(directionQueues[i]->front()))
                {
                    if(directionQueues[i]->front()->waited)
                    {
                        isSignaled = true;
                        directionsCopy.push_back(directionQueues[i]->front()->direction);
                        directionQueues[i]->front()->cv->notify_one();
                    }
                }
            }
            lock.unlock();
            if(isSignaled)
                this_thread::sleep_for(chrono::milliseconds(100));
        }
        else
        {
            lock.unlock();
            break;
        }
    }
}
vector<Car*> insertionSort(vector<Car*> &cars)
{
    for(int i = 0; i < cars.size(); i++)
    {
        Car min = *cars[i];
        int minIndex = i;
        for(int j = i + 1; j < cars.size(); j++)
        {
            if(cars[j]->arrivalTime < min.arrivalTime)
            {
                min = *cars[j];
                minIndex = j;
            }
        }
        if(minIndex != i)
        {
            Car temp = *cars[i];
            *cars[i] = *cars[minIndex];
            *cars[minIndex] = temp;
        }
    }
    return cars;
}

int main(int argc, char **argv)
{
    // BONUS: N cars in one direction can go
    if(argc == 3)
    {
        isConsecutive = true;
        maxConsecutive = stoi(argv[2]);
    }
    else if(argc != 2)
    {
        cout << "Enter a file path" << endl;
        exit(0);
    }
    
    vector<thread> threadList;
    int arrivalTime;
    string direction;
    
    // read file
    ifstream reader;
    reader.open(argv[1]);
    if(reader.fail())
    {
        cout << "File error" << endl;
        reader.close();
        exit(0);
    }
    else
    {
        while(!reader.eof())
        {
            reader >> arrivalTime >> direction;
            cars.push_back(new Car(arrivalTime, direction));
        }
        reader.close();
    }
        
    // sort cars by arrival time
    cars = insertionSort(cars);

    Intersection* intersection = new Intersection;
    int currentTime = 0;
    
    thread signaler = thread(&signalingThread);
    
    auto start = chrono::high_resolution_clock::now();
    
    for(int i = 0; i < cars.size(); i++)
    {
        while(cars[i]->arrivalTime != currentTime)
        {
            this_thread::sleep_for(chrono::seconds(1));
            currentTime++;
        }
        string output = "Created thread: " + to_string(cars[i]->arrivalTime) + " " + cars[i]->direction + "\n";
        cout << output;
        threadList.push_back(thread(&Intersection::tryTravel, intersection, cars[i]));
    }
    
    for(int i = 0; i < threadList.size(); i++)
        threadList[i].join();
    signaler.join();
     
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(stop - start);
    cout << "--------------------------------------------" << endl;
    cout << "Order of cars leaving intersection:" << endl;
    cout << finalOutput;
    cout << "Total time to pass intersection: " << duration.count() << " seconds" << endl;
}
