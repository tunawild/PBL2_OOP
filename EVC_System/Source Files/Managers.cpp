#include "Managers.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

using namespace std;

// Split string helper
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// --- UserManager ---
UserManager::UserManager() : loggedInUser(nullptr) {}

void UserManager::loadData() {
    admins.clear();
    customers.clear();
    ifstream fa("Data/admins.txt");
    if(fa.is_open()) {
        string line;
        while(getline(fa, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 3) admins.push_back(Admin(t[0], t[1], t[2]));
        }
    }
    ifstream fc("Data/customers.txt");
    if(fc.is_open()) {
        string line;
        while(getline(fc, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 10) customers.push_back(Customer(t[0], t[1], t[2], t[3], t[4], t[5], t[6], stof(t[7]), stof(t[8]), stoi(t[9])));
        }
    }
    // Add default admin if empty
    if(admins.empty()) admins.push_back(Admin("A01", "admin", "admin123"));
}

void UserManager::saveData() {
    ofstream fa("Data/admins.txt");
    for(auto& a : admins) fa << a.getId() << "|" << a.getUsername() << "|" << a.getPassword() << "\n";
    ofstream fc("Data/customers.txt");
    for(auto& c : customers) fc << c.getId() << "|" << c.getUsername() << "|" << c.getPassword() << "|" 
                                << c.getName() << "|" << c.getPhone() << "|" << c.getLicensePlate() << "|"
                                << c.getCarModel() << "|" << c.getBatteryCapacity() << "|" << c.getBalance() << "|" << c.getIsLocked() << "\n";
}

bool UserManager::login(const string& username, const string& password) {
    for(auto& a : admins) {
        if(a.getUsername() == username && a.checkPassword(password)) {
            loggedInUser = &a; return true;
        }
    }
    for(auto& c : customers) {
        if(c.getUsername() == username && c.checkPassword(password)) {
            if(c.getIsLocked()) return false; // locked
            loggedInUser = &c; return true;
        }
    }
    return false;
}

void UserManager::logout() { loggedInUser = nullptr; }
User* UserManager::getLoggedInUser() { return loggedInUser; }
vector<Customer>& UserManager::getCustomers() { return customers; }

Customer* UserManager::getCustomerById(const string& id) {
    for(auto& c : customers) if(c.getId() == id) return &c;
    return nullptr;
}

void UserManager::toggleCustomerLock(const string& id) {
    Customer* c = getCustomerById(id);
    if(c) c->setLocked(!c->getIsLocked());
}

void UserManager::addCustomer(const Customer& c) { customers.push_back(c); }

// --- StationManager ---
StationManager::StationManager() {}

void StationManager::loadData() {
    stations.clear();
    ifstream fs("Data/stations.txt");
    if(fs.is_open()) {
        string line;
        while(getline(fs, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 3) stations.push_back(ChargingStation(t[0], t[1], t[2]));
        }
    }
    ifstream fp("Data/ports.txt");
    if(fp.is_open()) {
        string line;
        while(getline(fp, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 7) {
                ChargingStation* st = getStationById(t[0]);
                if(st) {
                    st->addPort(ChargingPort(t[1], (PortType)stoi(t[2]), stof(t[3]), stof(t[5]), (PortStatus)stoi(t[4]), t[6]));
                }
            }
        }
    }
}

void StationManager::saveData() {
    ofstream fs("Data/stations.txt");
    ofstream fp("Data/ports.txt");
    for(auto& st : stations) {
        fs << st.stationId << "|" << st.name << "|" << st.address << "\n";
        for(auto& p : st.ports) {
            fp << st.stationId << "|" << p.portId << "|" << (int)p.type << "|" << p.powerKW << "|" 
               << (int)p.status << "|" << p.pricePerKWh << "|" << p.currentSessionId << "\n";
        }
    }
}

vector<ChargingStation>& StationManager::getStations() { return stations; }
ChargingStation* StationManager::getStationById(const string& id) {
    for(auto& s : stations) if(s.stationId == id) return &s;
    return nullptr;
}

void StationManager::addStation(const ChargingStation& st) { stations.push_back(st); }

bool StationManager::deleteStation(const string& id) {
    for(auto it = stations.begin(); it != stations.end(); ++it) {
        if(it->stationId == id) {
            for(auto& p : it->ports) {
                if(p.status == OCCUPIED) return false;
            }
            stations.erase(it);
            return true;
        }
    }
    return false;
}

void StationManager::updateStation(const string& id, const string& name, const string& address) {
    ChargingStation* st = getStationById(id);
    if(st) { st->name = name; st->address = address; }
}

void StationManager::addPortToStation(const string& stationId, const ChargingPort& port) {
    ChargingStation* st = getStationById(stationId);
    if(st) st->addPort(port);
}

void StationManager::updatePortStatus(const string& stationId, const string& portId, PortStatus newStatus) {
    ChargingStation* st = getStationById(stationId);
    if(st) {
        for(auto& p : st->ports) {
            if(p.portId == portId) p.status = newStatus;
        }
    }
}

void StationManager::updatePortPrice(const string& stationId, const string& portId, float newPrice) {
    ChargingStation* st = getStationById(stationId);
    if(st) {
        for(auto& p : st->ports) {
            if(p.portId == portId) p.pricePerKWh = newPrice;
        }
    }
}

int StationManager::getTotalStations() { return stations.size(); }
int StationManager::getTotalPorts() {
    int sum = 0;
    for(auto& st : stations) sum += st.ports.size();
    return sum;
}
int StationManager::getPortsByStatus(PortStatus status) {
    int sum = 0;
    for(auto& st : stations)
        for(auto& p : st.ports)
            if(p.status == status) sum++;
    return sum;
}

// --- SessionManager ---
SessionManager::SessionManager() {}

void SessionManager::loadData() {
    sessions.clear(); invoices.clear();
    ifstream fs("Data/sessions.txt");
    if(fs.is_open()) {
        string line;
        while(getline(fs, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 9) {
                ChargingSession ses(t[0], t[1], t[2], t[3], stoll(t[4]));
                ses.endTime = stoll(t[5]);
                ses.energyKWh = stof(t[6]);
                ses.cost = stof(t[7]);
                ses.isActive = stoi(t[8]);
                sessions.push_back(ses);
            }
        }
    }
    ifstream fi("Data/invoices.txt");
    if(fi.is_open()) {
        string line;
        while(getline(fi, line)) {
            vector<string> t = split(line, '|');
            if(t.size() >= 5) {
                invoices.push_back(Invoice(t[0], t[1], t[2], stof(t[3]), stoll(t[4])));
            }
        }
    }
}

void SessionManager::saveData() {
    ofstream fs("Data/sessions.txt");
    for(auto& s : sessions) {
        fs << s.sessionId << "|" << s.customerId << "|" << s.stationId << "|" << s.portId << "|" 
           << s.startTime << "|" << s.endTime << "|" << s.energyKWh << "|" << s.cost << "|" << s.isActive << "\n";
    }
    ofstream fi("Data/invoices.txt");
    for(auto& i : invoices) {
        fi << i.invoiceId << "|" << i.sessionId << "|" << i.customerId << "|" << i.totalAmount << "|" << i.date << "\n";
    }
}

vector<ChargingSession>& SessionManager::getSessions() { return sessions; }
vector<Invoice>& SessionManager::getInvoices() { return invoices; }

bool SessionManager::startSession(const string& customerId, const string& stationId, const string& portId, StationManager& sm) {
    ChargingStation* st = sm.getStationById(stationId);
    if(!st) return false;
    for(auto& p : st->ports) {
        if(p.portId == portId && p.status == AVAILABLE) {
            string sId = "SES_" + to_string(time(0));
            ChargingSession ses(sId, customerId, stationId, portId, time(0));
            sessions.push_back(ses);
            p.status = OCCUPIED;
            p.currentSessionId = sId;
            return true;
        }
    }
    return false;
}

bool SessionManager::endSession(const string& customerId, StationManager& sm, UserManager& um) {
    for(auto& s : sessions) {
        if(s.isActive && s.customerId == customerId) {
            long long endT = time(0);
            long long duration = endT - s.startTime;
            if(duration <= 0) duration = 1; 

            ChargingStation* st = sm.getStationById(s.stationId);
            if(st) {
                for(auto& p : st->ports) {
                    if(p.portId == s.portId) {
                        float energy = (p.powerKW * (duration / 3600.0f)); // kWh
                        if (energy < 0.1f) energy = 0.5f; // min charge for demo
                        float totalCost = energy * p.pricePerKWh;
                        
                        Customer* c = um.getCustomerById(customerId);
                        if(c && c->getBalance() >= totalCost) {
                            c->deductBalance(totalCost);
                            s.endSession(endT, energy, totalCost);
                            p.status = AVAILABLE;
                            p.currentSessionId = "";
                            
                            string invId = "INV_" + to_string(time(0));
                            invoices.push_back(Invoice(invId, s.sessionId, customerId, totalCost, endT));
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

float SessionManager::getTotalRevenue() {
    float sum = 0;
    for(auto& i : invoices) sum += i.totalAmount;
    return sum;
}
float SessionManager::getTotalEnergyConsumed() {
    float sum = 0;
    for(auto& s : sessions) sum += s.energyKWh;
    return sum;
}

// --- DataManager ---
void DataManager::loadAll(UserManager& um, StationManager& sm, SessionManager& sem) {
    um.loadData();
    sm.loadData();
    sem.loadData();
}

void DataManager::saveAll(UserManager& um, StationManager& sm, SessionManager& sem) {
    um.saveData();
    sm.saveData();
    sem.saveData();
}
