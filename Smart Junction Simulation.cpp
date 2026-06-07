// SMART JUNCTION SIMULATION  
// A smart traffic control simulation system for adaptive junction management and real-time decision making
   
#include<iostream>
#include<queue>
#include<string>
#include<vector>
#include<algorithm>
#include<chrono>
#include<thread>
#include<fstream>
#include<cstdio>

using namespace std;

//  FORWARD DECLARATIONS
class Vehicle;
class Road;
class Junction;
class Car;
class Bike;
class Truck;
class Ambulance;

//  TEMPLATE FUNCTION 
template <typename T>
T getMax(T a, T b)
{
    return (a > b) ? a : b;
}

//  NAMESPACE 
namespace TrafficUtils                          //  Static Function 
{
    static int calculateFine(int overSpeed)
    {
        static int fine;                        //  Static Variable
        fine=overSpeed * 100;
        return fine;
    }
}

//  ENUM CLASS
enum class Priority { NORMAL, EMERGENCY };

//  COUNTDOWN
inline void countdown(int seconds)              // Inline Function
{
    while (seconds > 0)
    {
        cout << seconds << " ";
        this_thread::sleep_for(chrono::milliseconds(300));
        seconds--;
    }
    cout << endl;
}


//  BASE CLASS
class Vehicle
{
protected:
    string type;
    int speed;
    string number;

public:
    
    //  Function Overloading

    Vehicle() : type("Unknown"), speed(0), number("NA") {}                  //  default constructor 
    Vehicle(string t, int s, string n) : type(t), speed(s), number(n) {}    // parameterized constructor 
    Vehicle(const Vehicle& v)                                               // copy constructor
    {
        type = v.type;
        speed = v.speed;
        number = v.number;
    }

    virtual void move() = 0;        // pure virtual function

    virtual Priority getPriority()  
    {
        return Priority::NORMAL;
    }

    string getType() { return type; }
    int getSpeed() { return speed; }
    string getNumber() { return number; }

    virtual ~Vehicle() {}

    bool operator>(Vehicle& v)          //  Opertor Overloading
    {
        return this->speed > v.speed;
    }

    friend void showVehicle(Vehicle& v);
};

//  FRIEND FUNCTION
void showVehicle(Vehicle& v)    
{
    cout << v.number << " (" << v.type << ")";
}

//  DERIVED
class Car : public Vehicle {
public:
    Car(int s, string n) : Vehicle("Car", s, n) {}
    void move() override {} // intentionally left blank for cleaner output
};

class Bike : public Vehicle {
public:
    Bike(int s, string n) : Vehicle("Bike", s, n) {}
    void move() override {}
};

class Truck : public Vehicle {
public:
    Truck(int s, string n) : Vehicle("Truck", s, n) {}
    void move() override {}
};

class Ambulance : public Vehicle {
public:
    Ambulance(string n) : Vehicle("Ambulance", 100, n) {}
    void move() override {}
    Priority getPriority() override { return Priority::EMERGENCY; }
};


//  Traffic Light
class TrafficLight
{
    string state;
public:
    void setState(string s) { state = s; }
    string getState() { return state; }
};

//  Road
class Road
{
    string name;
    queue<Vehicle*> vehicles;
    string signalState;

public:
    Road(string n) : name(n), signalState("RED") {}

    void addVehicle(Vehicle* v)
    {
        vehicles.push(v);
    }

    queue<Vehicle*>& getQueue() { return vehicles; }

    string getName() { return name; }

    void setSignal(string state) { signalState = state; }

    void displayStatus()
    {
        cout << name << "->" << signalState << endl;
    }
};

//  Junction
class Junction
{
    vector<Road> roads;

public:
    Junction()
    {
        roads.push_back(Road("North"));
        roads.push_back(Road("South"));
        roads.push_back(Road("East"));
        roads.push_back(Road("West"));
    }

    vector<Road>& getRoads() { return roads; }
};

// OVERSPEED CHECK
void checkOverspeed(Vehicle* v)
{
    ofstream file("fines.txt", ios::app);

    int limit = 0;

    if (v->getType() == "Car") limit = 80;
    else if (v->getType() == "Bike") limit = 60;
    else if (v->getType() == "Truck") limit = 50;
    else return;

 
    int maxVal = getMax(v->getSpeed(), limit);
    if (maxVal == v->getSpeed() && v->getSpeed() > limit)
    {
        //  static_cast added
        int fine = static_cast<int>(TrafficUtils::calculateFine(v->getSpeed() - limit));

        file << "Vehicle: " << v->getNumber()
             << " | Type: " << v->getType()
             << " | Fine: " << fine << endl;
    }
    file.close();
}

// DISPLAY FINES 
void displayFines()
{
    ifstream file("fines.txt");

    if (!file)
    {
        cout << "\nNo fines recorded yet.\n";
        return;
    }

    cout << "\n===== FINES =====\n";

    string line;
    bool empty = true;

    while (getline(file, line))
    {
        cout << line << endl;
        empty = false;
    }

    if (empty)
        cout << "No violations recorded.\n";
}

//  FILE INPUT
void loadVehiclesFromFile(Junction& junction, string filename)
{
    ifstream file(filename);

    if (!file)
    {
        throw runtime_error("Error: Cannot open file!");    // Exception Handling
    }

    string header;
    getline(file, header);

    int r, t, s;
    string num;

    while (file >> r >> t >> s >> num)
    {
        Road& road = junction.getRoads()[r - 1];

        if (t == 1) road.addVehicle(new Car(s, num));
        else if (t == 2) road.addVehicle(new Bike(s, num));
        else if (t == 3) road.addVehicle(new Truck(s, num));
        else if (t == 4) road.addVehicle(new Ambulance(num));
    }

    cout << "Loaded from file\n";
}

//  CLEAR JUNCTION 
void clearJunction(Junction& junction)
{
    for (auto &road : junction.getRoads())
    {
        auto &q = road.getQueue();
        while (!q.empty())
        {
            delete q.front();
            q.pop();
        }
    }
}

//  Controller
class TrafficController
{
    TrafficLight* light;

    void runSignal(Road& road, string state, int sec, bool timer)
    {
        light->setState(state);
        road.setSignal(state);

        cout << "\n" << road.getName() << "->" << state;

        if (timer)
        {
            cout << " ";
            countdown(sec);
        }
        else cout << endl;
    }

    bool hasAmbulance(queue<Vehicle*> q)
    {
        while (!q.empty())
        {
            //  dynamic_cast 
            Ambulance* amb = dynamic_cast<Ambulance*>(q.front());
            if (amb) return true;

            q.pop();
        }
        return false;
    }

public:
    TrafficController()
    {
        light = new TrafficLight();
    }

    void manage(Junction& j)
    {
        vector<Road*> order;

        for (auto& r : j.getRoads())
            if (hasAmbulance(r.getQueue()))
                order.push_back(&r);

        vector<Road*> rem;
        for (auto& r : j.getRoads())
            rem.push_back(&r);

        sort(rem.begin(), rem.end(),
            [](Road* a, Road* b)
            {
                return a->getQueue().size() > b->getQueue().size();
            });

        order.insert(order.end(), rem.begin(), rem.end());

        for (auto road : order)
        {
            

            auto& q = road->getQueue();
            if (q.empty()) continue;

            cout << "\n=== " << road->getName() << " ===\n";
            runSignal(*road,"GREEN", 10, true);
            runSignal(*road,"YELLOW", 5, true);
            runSignal(*road,"RED", 0, false);

            while (!q.empty())
            {
                q.front()->move();
                checkOverspeed(q.front());
                delete q.front();
                q.pop();
            }
        }
    }

    ~TrafficController()
    {
        delete light;
    }
};

//  MAIN
int main()
{
    Junction junction;

    while (true)
    {
        cout << "\n===== JUNCTION CONTROL ======\n";
        cout << "| 1. Add Vehicle Manually     |\n";
        cout << "| 2. Load Vehicles From File  |\n";
        cout << "| 3. Run Simulation           |\n";
        cout << "| 4. Display Junction Status  |\n";
        cout << "| 5. View Fines               |\n";
        cout << "| 6. Exit                     |\n";
        cout << "==============================\n";

        int ch;
        cout << "Enter choice: ";
        cin >> ch;

        if (ch == 1)
        {
            int r, v, speed;
            string num;
            cout << "Enter road (1-North, 2-South, 3-East, 4-West): ";
            cin >> r;
            cout << "Enter vehicle type (1-Car, 2-Bike, 3-Truck, 4-Ambulance): ";
            cin >> v;
            cout << "Enter vehicle number: ";
            cin >> num;
            if (v != 4) 
            {
                cout << "Enter speed: ";
                cin >> speed;
            }

            Road& road = junction.getRoads()[r - 1];

            if (v == 1) road.addVehicle(new Car(speed, num));
            else if (v == 2) road.addVehicle(new Bike(speed, num));
            else if (v == 3) road.addVehicle(new Truck(speed, num));
            else if (v == 4) road.addVehicle(new Ambulance(num));
        }
        else if (ch == 2)
        {
            string filename;
            cout << "Enter filename: ";
            cin >> filename;

            try // Exception Handling
            {
                clearJunction(junction);
                loadVehiclesFromFile(junction, filename);
            }
            catch (exception &e)
            {
                cout << e.what() << endl;
            }
        }
        else if (ch == 3)
        {
            ofstream clearFile("fines.txt");
            clearFile.close();

            TrafficController controller;
            controller.manage(junction);
        }
        else if (ch == 4)
        {
            for (auto &road : junction.getRoads())
                road.displayStatus();
        }
        else if (ch == 5)
        {
            displayFines();
        }
        else if (ch == 6)
        {
            remove("fines.txt");
            clearJunction(junction);
            break;
        }
    }
}