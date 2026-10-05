#include "UI.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <string.h>
#include <iostream>

using namespace std;

AppUI::AppUI(UserManager& u, StationManager& s, SessionManager& se)
    : um(u), sm(s), sem(se), currentState(STATE_LOGIN), loginUsernameActive(false), loginPasswordActive(false), loginError(false), adminMenuSelected(0), customerMenuSelected(0) {
    memset(loginUsername, 0, sizeof(loginUsername));
    memset(loginPassword, 0, sizeof(loginPassword));
    memset(newStationName, 0, sizeof(newStationName));
    memset(newStationAddress, 0, sizeof(newStationAddress));
    memset(customerSearch, 0, sizeof(customerSearch));
}

void AppUI::Init() {
    GuiLoadStyleDefault();
}

void AppUI::UpdateDrawFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (currentState) {
        case STATE_LOGIN: DrawLogin(); break;
        case STATE_ADMIN_DASHBOARD: DrawAdminDashboard(); break;
        case STATE_ADMIN_STATIONS: DrawAdminStations(); break;
        case STATE_ADMIN_CUSTOMERS: DrawAdminCustomers(); break;
        case STATE_ADMIN_INVOICES: DrawAdminInvoices(); break;
        case STATE_CUSTOMER_MENU: DrawCustomerMenu(); break;
        case STATE_CUSTOMER_CHARGING: DrawCustomerCharging(); break;
    }

    EndDrawing();
}

void AppUI::DrawLogin() {
    DrawText("EV CHARGING STATION MANAGEMENT", 200, 100, 30, DARKBLUE);
    
    if (GuiTextBox((Rectangle){ 350, 200, 200, 30 }, loginUsername, 64, loginUsernameActive)) {
        loginUsernameActive = !loginUsernameActive;
        if (loginUsernameActive) loginPasswordActive = false;
    }
    if (GuiTextBox((Rectangle){ 350, 250, 200, 30 }, loginPassword, 64, loginPasswordActive)) {
        loginPasswordActive = !loginPasswordActive;
        if (loginPasswordActive) loginUsernameActive = false;
    }
    
    DrawText("Username:", 250, 205, 20, DARKGRAY);
    DrawText("Password:", 250, 255, 20, DARKGRAY);

    if (GuiButton((Rectangle){ 400, 300, 100, 40 }, "LOGIN")) {
        if(um.login(loginUsername, loginPassword)) {
            loginError = false;
            if(um.getLoggedInUser()->getRole() == ROLE_ADMIN) {
                currentState = STATE_ADMIN_DASHBOARD;
            } else {
                currentState = STATE_CUSTOMER_MENU;
            }
        } else {
            loginError = true;
        }
    }

    if(loginError) {
        DrawText("Invalid username or password (or locked)!", 300, 360, 20, RED);
    }
}

void AppUI::DrawAdminSidebar() {
    DrawRectangle(0, 0, 200, GetScreenHeight(), LIGHTGRAY);
    DrawText("ADMIN PANEL", 20, 20, 20, DARKBLUE);

    if (GuiButton((Rectangle){ 10, 80, 180, 40 }, "Dashboard")) { currentState = STATE_ADMIN_DASHBOARD; }
    if (GuiButton((Rectangle){ 10, 130, 180, 40 }, "Stations & Ports")) { currentState = STATE_ADMIN_STATIONS; }
    if (GuiButton((Rectangle){ 10, 180, 180, 40 }, "Customers")) { currentState = STATE_ADMIN_CUSTOMERS; }
    if (GuiButton((Rectangle){ 10, 230, 180, 40 }, "Invoices & Sessions")) { currentState = STATE_ADMIN_INVOICES; }
    
    if (GuiButton((Rectangle){ 10, GetScreenHeight() - 60, 180, 40 }, "LOGOUT")) {
        um.logout();
        currentState = STATE_LOGIN;
        memset(loginUsername, 0, 64);
        memset(loginPassword, 0, 64);
    }
}

void AppUI::DrawAdminDashboard() {
    DrawAdminSidebar();
    DrawText("DASHBOARD", 250, 20, 30, DARKGRAY);

    int totalStations = sm.getTotalStations();
    int totalPorts = sm.getTotalPorts();
    int avail = sm.getPortsByStatus(AVAILABLE);
    int occ = sm.getPortsByStatus(OCCUPIED);
    int maint = sm.getPortsByStatus(MAINTENANCE);
    float revenue = sem.getTotalRevenue();
    float energy = sem.getTotalEnergyConsumed();

    DrawText(TextFormat("Total Stations: %d", totalStations), 250, 100, 20, BLACK);
    DrawText(TextFormat("Total Ports: %d", totalPorts), 250, 140, 20, BLACK);
    DrawText(TextFormat(" - Available: %d", avail), 270, 180, 20, GREEN);
    DrawText(TextFormat(" - Occupied: %d", occ), 270, 220, 20, ORANGE);
    DrawText(TextFormat(" - Maintenance: %d", maint), 270, 260, 20, RED);
    
    DrawText(TextFormat("Total Revenue: %.2f VND", revenue), 250, 320, 20, BLUE);
    DrawText(TextFormat("Total Energy: %.2f kWh", energy), 250, 360, 20, BLUE);
}

void AppUI::DrawAdminStations() {
    DrawAdminSidebar();
    DrawText("STATION MANAGEMENT", 250, 20, 30, DARKGRAY);
    // Simple placeholder for stations
    DrawText("List of Stations:", 250, 80, 20, BLACK);
    int y = 120;
    for(auto& st : sm.getStations()) {
        DrawText(TextFormat("ID: %s - %s - %s (Ports: %d)", st.stationId.c_str(), st.name.c_str(), st.address.c_str(), st.ports.size()), 250, y, 20, DARKGRAY);
        y += 30;
    }
}

void AppUI::DrawAdminCustomers() {
    DrawAdminSidebar();
    DrawText("CUSTOMER MANAGEMENT", 250, 20, 30, DARKGRAY);
    int y = 100;
    for(auto& c : um.getCustomers()) {
        DrawText(TextFormat("ID: %s - %s - %s - Bal: %.0f", c.getId().c_str(), c.getName().c_str(), c.getLicensePlate().c_str(), c.getBalance()), 250, y, 20, DARKGRAY);
        if(GuiButton((Rectangle){ 800, (float)y, 80, 25 }, c.getIsLocked() ? "UNLOCK" : "LOCK")) {
            um.toggleCustomerLock(c.getId());
        }
        y += 35;
    }
}

void AppUI::DrawAdminInvoices() {
    DrawAdminSidebar();
    DrawText("INVOICES & SESSIONS", 250, 20, 30, DARKGRAY);
    int y = 100;
    for(auto& inv : sem.getInvoices()) {
        DrawText(TextFormat("Inv: %s - Cust: %s - Amt: %.0f VND", inv.invoiceId.c_str(), inv.customerId.c_str(), inv.totalAmount), 250, y, 20, DARKGRAY);
        y += 30;
    }
}

void AppUI::DrawCustomerMenu() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if(!c) return;

    // Header
    DrawRectangle(0, 0, GetScreenWidth(), 60, DARKBLUE);
    DrawText(TextFormat("Welcome, %s | ID: %s | Bal: %.0f VND", c->getName().c_str(), c->getId().c_str(), c->getBalance()), 20, 20, 20, WHITE);
    
    if (GuiButton((Rectangle){ (float)GetScreenWidth() - 100, 10, 80, 40 }, "LOGOUT")) {
        um.logout();
        currentState = STATE_LOGIN;
        memset(loginUsername, 0, 64);
        memset(loginPassword, 0, 64);
    }

    DrawText("AVAILABLE STATIONS", 50, 100, 30, DARKGRAY);
    
    int y = 150;
    for(auto& st : sm.getStations()) {
        DrawText(TextFormat("Station: %s (%s)", st.name.c_str(), st.address.c_str()), 50, y, 20, BLACK);
        y += 30;
        for(auto& p : st.ports) {
            Color cColor = (p.status == AVAILABLE) ? GREEN : ((p.status == OCCUPIED) ? ORANGE : RED);
            string statusStr = (p.status == AVAILABLE) ? "AVAILABLE" : ((p.status == OCCUPIED) ? "OCCUPIED" : "MAINTENANCE");
            DrawText(TextFormat(" - Port %s: %s (%.0fkW) - %.0f VND/kWh", p.portId.c_str(), statusStr.c_str(), p.powerKW, p.pricePerKWh), 70, y, 20, cColor);
            
            if(p.status == AVAILABLE) {
                if(GuiButton((Rectangle){ 550, (float)y, 100, 25 }, "START CHARGE")) {
                    if(sem.startSession(c->getId(), st.stationId, p.portId, sm)) {
                        currentState = STATE_CUSTOMER_CHARGING;
                    }
                }
            }
            y += 30;
        }
        y += 20;
    }
}

void AppUI::DrawCustomerCharging() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if(!c) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), LIGHTGRAY);
    DrawText("CHARGING IN PROGRESS...", 300, 200, 30, ORANGE);

    if (GuiButton((Rectangle){ 400, 300, 200, 50 }, "STOP CHARGING & PAY")) {
        if(sem.endSession(c->getId(), sm, um)) {
            currentState = STATE_CUSTOMER_MENU;
        }
    }
}
