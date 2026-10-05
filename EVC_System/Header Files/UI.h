#pragma once
#include "Managers.h"
#include "raylib.h"

enum AppState {
    STATE_LOGIN,
    STATE_ADMIN_DASHBOARD,
    STATE_ADMIN_STATIONS,
    STATE_ADMIN_CUSTOMERS,
    STATE_ADMIN_INVOICES,
    STATE_CUSTOMER_MENU,
    STATE_CUSTOMER_CHARGING
};

class AppUI {
private:
    UserManager& um;
    StationManager& sm;
    SessionManager& sem;
    AppState currentState;

    // Login screen state
    char loginUsername[64];
    char loginPassword[64];
    bool loginUsernameActive;
    bool loginPasswordActive;
    bool loginError;

    // Admin state
    int adminMenuSelected;
    
    // Stations screen state
    char newStationName[128];
    char newStationAddress[128];
    
    // Customer screen state
    char customerSearch[128];

    // Customer Menu state
    int customerMenuSelected;

public:
    AppUI(UserManager& u, StationManager& s, SessionManager& se);
    void Init();
    void UpdateDrawFrame();

private:
    void DrawLogin();
    void DrawAdminDashboard();
    void DrawAdminStations();
    void DrawAdminCustomers();
    void DrawAdminInvoices();
    
    void DrawCustomerMenu();
    void DrawCustomerCharging();

    void DrawAdminSidebar();
};
