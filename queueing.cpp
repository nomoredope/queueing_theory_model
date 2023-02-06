#include <iostream>
#include <queue>
#include <fstream>
#include <ctime>
#include <algorithm>

using namespace std;

double random(double min, double max) {
    double res;
    res = (double) rand() / RAND_MAX * (max - min) + min;
    return res;
}

enum TransactConditions {
    ARRIVING,
    IN_SERVICE,
    IN_QUEUE,
};

enum DeviceConditions {
    FREE,
    BUSY,
};

class Transact {
private:
    TransactConditions condition;
    int device_id;
    int id;
    double nextActionTime;
public:
    Transact(int _id, double _tg) {
        id = _id;
        condition = TransactConditions::ARRIVING;
        nextActionTime = _tg;
        device_id = -1;
    }
    int getId() const { return id; }
    int getDeviceId() const { return device_id; }
    double getNextActionTime() const { return nextActionTime; }
    void setNextActionTime(double _nat) { nextActionTime = _nat; }
    void setCondition(TransactConditions _condition) { condition = _condition; }
    void setDeviceId(int _did) { device_id = _did; }
    TransactConditions getCondition() { return condition; }
    void print() const;
    void exit() const;
};

void Transact::print() const {
    cout << "Id: " << id << endl;
    if (condition == IN_QUEUE) cout << "Condition: IN QUEUE to the desk " << device_id << endl;
    if (condition == IN_SERVICE) cout << "Condition: IN SERVICE by desk " << device_id << endl;
    if (condition == ARRIVING) cout << "Condition: ARRIVING" << endl;
    cout << "Next action time: " << nextActionTime << endl;
}

class Queue {
private:
    queue<Transact *> deviceQueue;
    int length;
public:
    Queue() {
        length = 0;
    }
    Transact *addToQueue(Transact *transact);
    void moveQueue();
    Transact *getFirst() { return deviceQueue.front(); }
    int getLength() const { return length; }
};

class Device {
private:
    Transact *currentTransact;
    int deviceId;
    double minServiceTime;
    double maxServiceTime;
    DeviceConditions condition;
public:
    Queue *deviceQueue;
    Device(int _dId, double _minST, double _maxST, Queue *_q) {
        deviceId = _dId;
        maxServiceTime = _maxST, minServiceTime = _minST;
        condition = DeviceConditions::FREE;
        currentTransact = nullptr;
        deviceQueue = _q;
    }
    DeviceConditions getCondition() { return condition; }
    void free() { condition = FREE; }
    Transact *enterDevice(Transact *transact);
    ~Device() { delete currentTransact; }
};

Transact *Device::enterDevice(Transact *transact) {
    ofstream out;
    out.open("simulation_log.txt", ios::app);
    currentTransact = transact;
    currentTransact->setDeviceId(this->deviceId);
    currentTransact->setCondition(TransactConditions::IN_SERVICE);
    condition = DeviceConditions::BUSY;
    currentTransact->setNextActionTime(currentTransact->getNextActionTime() + random(minServiceTime, maxServiceTime));
    out.close();
    return currentTransact;
}

void Transact::exit() const {
    ofstream out;
    out.open("simulation_log.txt", ios::app);
    out << "Transact with id " << id << " left model, time is: " << nextActionTime << endl;
    out.close();
}

Transact *Queue::addToQueue(Transact *transact) {
    transact->setCondition(TransactConditions::IN_QUEUE);
    deviceQueue.push(transact);
    length++;
    return transact;
}

void Queue::moveQueue() {
    deviceQueue.pop();
    length--;
}

bool comp(Transact *tr1, Transact *tr2) {
    return tr1->getNextActionTime() < tr2->getNextActionTime();
}

int main() {
    double r1, g1, b1;
    double simulationTime;
    ofstream out;
    double mq1 = 0;
    double mq2 = 0;
    double k1 = 0;
    double k2 = 0;
    out.open("simulation_log.txt", ios::app);
    time_t now = time(nullptr);
    tm *gmtm = localtime(&now);
    out << "******************************************************" << endl;
    out << "Started simulation at: " << asctime(gmtm) << endl;
    out << "******************************************************" << endl;
    cout << "Input r1, g1, b1" << endl;
    cin >> r1 >> g1 >> b1;
    cout << "Input simulation time" << endl;
    cin >> simulationTime;
    srand(time(nullptr));
    double currentTime;
    Device *desks[2];
    auto *queueToDesk1 = new Queue();
    auto *queueToDesk2 = new Queue();
    desks[0] = new Device(1, r1, r1 + g1 + b1, queueToDesk1);
    desks[1] = new Device(2, g1, r1 + g1 + b1, queueToDesk2);
    vector<Transact *> FEC;
    vector<Transact *> CEC;
    int amount = 0;
    FEC.push_back(new Transact(1, random(0, r1 + g1 + b1)));
    currentTime = FEC[0]->getNextActionTime();
    while (currentTime < simulationTime) {
        for (auto &i : FEC) {
            if (i->getNextActionTime() == currentTime) {
                CEC.push_back(i);
            }
        }
        cout << "******************************************************" << endl;
        cout << "Time is: " << currentTime << endl;
        cout << "Total objects in CEC:" << CEC.size() << endl;
        for (auto &i : CEC) {
            i->print();
        }
        for (int i = 0; i < CEC.size(); i++) {
            if (CEC[i]->getCondition() == TransactConditions::ARRIVING) {
                amount += 1;
                out << "Transact with id " << amount << " enters model at " << currentTime << endl;
                mq1 += queueToDesk1->getLength();
                mq2 += queueToDesk2->getLength();
                k1 += 1;
                k2 += 2;
                if (desks[0]->getCondition() == DeviceConditions::FREE) {
                    out << "Transact with id " << CEC[i]->getId() << " enters desk number 1 at " << currentTime << endl;
                    FEC[i] = desks[0]->enterDevice(CEC[i]);
                } else if (desks[1]->getCondition() == DeviceConditions::FREE) {
                    out << "Transact with id " << CEC[i]->getId() << " enters desk number 2 at " << currentTime << endl;
                    FEC[i] = desks[1]->enterDevice(CEC[i]);
                } else if (queueToDesk1->getLength() <= queueToDesk2->getLength()) {
                    out << "Transact with id " << CEC[i]->getId() << " enters queue to the desk number 1 at " << currentTime << endl;
                    queueToDesk1->addToQueue(CEC[i]);
                    FEC.erase(FEC.cbegin() + i);
                } else {
                    out << "Transact with id " << CEC[i]->getId() << " enters queue to the desk number 2 at " << currentTime << endl;
                    queueToDesk2->addToQueue(CEC[i]);
                    FEC.erase(FEC.cbegin() + i);
                }
                FEC.push_back(new Transact(amount + 1, currentTime + random(0, r1 + g1 + b1)));
            } else if (CEC[i]->getCondition() == TransactConditions::IN_SERVICE) {
                int dd = CEC[i]->getDeviceId() - 1;
                out << "Transact with id " << CEC[i]->getId() << " frees desk number " << dd + 1 << " at "
                    << currentTime << endl;
                CEC[i]->exit();
                desks[dd]->free();
                if (desks[dd]->deviceQueue->getLength() > 0) {
                    out << "Transact with id " << desks[dd]->deviceQueue->getFirst()->getId() << " enters desk number " << dd + 1
                        << " at "
                        << currentTime << endl;
                    desks[dd]->deviceQueue->getFirst()->setNextActionTime(currentTime);
                    FEC[i] = desks[dd]->enterDevice(desks[dd]->deviceQueue->getFirst());
                    desks[dd]->deviceQueue->moveQueue();
                } else FEC.erase(FEC.cbegin() + i);
            }
        }
        sort(FEC.begin(), FEC.end(), comp);
        cout << "******************************************************" << endl;
        cout << "Total objects in FEC:" << FEC.size() << endl;
        cout << "Time is: " << currentTime << endl;
        cout << "Transacts in queue to desk1: " << queueToDesk1->getLength() << endl;
        cout << "Transacts in queue to desk2: " << queueToDesk2->getLength() << endl;
        for (auto &i : FEC) {
            i->print();
        }
        cout << "******************************************************" << endl;
        CEC.clear();
        currentTime = FEC[0]->getNextActionTime();
    }
    out << "******************************************************" << endl;
    out.close();
    return 0;
}