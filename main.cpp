#include "raylib.h"
#include "Managers.h"
#include "UI.h"

/**
 * @brief Ham main khoi dong chuong trinh Quan ly tram sac xe dien (PBL2 - OOP).
 * - Nap toan bo du lieu hoan toan tu cac tep CSV (khong khoi tao gia tri mac dinh).
 * - Khoi tao cua so do hoa va vong lap giao dien Raylib (1024x768).
 * - Luu toan bo du lieu xuong cac tep CSV truoc khi dong chuong trinh.
 */
int main() {
    // 1. Khoi tao cac lop quan ly nghiep vu (Managers)
    UserManager userManager;
    StationManager stationManager;
    SessionManager sessionManager;

    // 2. Nap du lieu tu cac tep CSV (admins, customers, stations, ports, sessions, invoices)
    // Hoan toan su dung du lieu co trong cac file CSV trong thu muc Data/
    DataManager::loadAll(userManager, stationManager, sessionManager);

    // 3. Khoi tao cua so giao dien do hoa Raylib
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "EV Charging Station Management - PBL2 OOP");
    SetTargetFPS(60);

    // 4. Khoi tao lop dieu phoi giao dien AppUI
    AppUI appUI(userManager, stationManager, sessionManager);
    appUI.Init();

    // 5. Vong lap tro choi / ung dung chinh (Main Application Loop)
    while (!WindowShouldClose()) {
        appUI.UpdateDrawFrame();
    }

    // 6. Tu dong luu toan bo du lieu vao cac tep CSV truoc khi thoat
    DataManager::saveAll(userManager, stationManager, sessionManager);

    // 7. Giai phong tai nguyen va dong cua so do hoa
    CloseWindow();
    return 0;
}
