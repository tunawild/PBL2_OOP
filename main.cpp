#include "raylib.h"
#include "Managers.h"
#include "UI.h"
#include <iostream>

int main() {
    // 1. Initialize Managers
    UserManager userManager;
    StationManager stationManager;
    SessionManager sessionManager;

    // Load data from files (if any)
    DataManager::loadAll(userManager, stationManager, sessionManager);

    // If no stations exist (first run), create some dummy data
    if(stationManager.getTotalStations() == 0) {
        ChargingStation s1("ST01", "Tram TTX", "Hoa Khanh, Da Nang");
        s1.addPort(ChargingPort("P01", AC_STANDARD, 22.0f, 3500.0f));
        s1.addPort(ChargingPort("P02", DC_FAST, 50.0f, 4500.0f));
        stationManager.addStation(s1);

        ChargingStation s2("ST02", "Tram Vincom", "Ngo Quyen, Da Nang");
        s2.addPort(ChargingPort("P03", DC_SUPERCHARGE, 150.0f, 6000.0f));
        stationManager.addStation(s2);
    }
    
    // If no customers exist, create a dummy customer
    if(userManager.getCustomers().empty()) {
        userManager.addCustomer(Customer("C01", "khach1", "123", "Nguyen Van A", "090123456", "43A-12345", "VF8", 82.0f, 500000.0f));
    }

    // 2. Initialize UI
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "EV Charging Station Management");
    SetTargetFPS(60);

    AppUI appUI(userManager, stationManager, sessionManager);
    appUI.Init();

    // 3. Main game loop
    while (!WindowShouldClose()) {
        appUI.UpdateDrawFrame();
    }

    // 4. Save data before exit
    DataManager::saveAll(userManager, stationManager, sessionManager);

    CloseWindow();
    return 0;
}
