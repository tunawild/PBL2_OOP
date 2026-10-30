#include "raylib.h"
#include "Managers.h"
#include "UI.h"

/**
 * @brief Ham main khoi dong chuong trinh Quan ly tram sac xe dien (PBL2 - OOP).
 * - Nap toan bo du lieu hoan toan tu cac tep CSV (khong khoi tao gia tri mac dinh).
 * - Khoi tao cua so do hoa toan man hinh (InitWindow(0, 0) khong dung co MAXIMIZED de tranh loi lech Viewport/Hitbox).
 * - Luu toan bo du lieu xuong cac tep CSV truoc khi dong chuong trinh.
 */
int main() {
    // 1. Khoi tao cac lop quan ly nghiep vu (Managers)
    UserManager userManager;
    StationManager stationManager;
    SessionManager sessionManager;

    // 2. Nap du lieu tu cac tep CSV (admins, customers, stations, ports, sessions, invoices)
    DataManager::loadAll(userManager, stationManager, sessionManager);

    // 3. Bat co ho tro thay doi kich thuoc va khu rang cua 4x (MSAA)
    // LUU Y QUAN TRONG: Khong bat FLAG_WINDOW_MAXIMIZED truoc InitWindow
    // vi se gay loi lech OpenGL Viewport va lech hitbox chuot tren Windows.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    // Khoi tao cua so co so 1280x720, dat kich thuoc toi thieu 1024x600
    InitWindow(1280, 720, "EV Charging Station Management - PBL2 OOP");
    SetWindowMinSize(1024, 600);

    // Phong to toan man hinh (MaximizeWindow) SAU khi cua so da duoc khoi tao
    // Giup GLFW va Raylib tu dong tinh toan chinh xac Render Size, Screen Size
    // va Viewport khop 100% voi do phan giai thuc te cua man hinh may tinh,
    // dong thoi giu hitbox chuot chinh xac tuyet doi 1:1.
    MaximizeWindow();
    SetTargetFPS(60);

    // 4. Khoi tao lop dieu phoi giao dien AppUI va nap phong chu Segoe UI
    AppUI appUI(userManager, stationManager, sessionManager);
    appUI.Init();

    // 5. Vong lap ung dung chinh (Main Application Loop)
    while (!WindowShouldClose()) {
        appUI.UpdateDrawFrame();
    }

    // 6. Tu dong luu toan bo du lieu vao cac tep CSV truoc khi thoat
    DataManager::saveAll(userManager, stationManager, sessionManager);

    // 7. Giai phong tai nguyen va dong cua so do hoa
    CloseWindow();
    return 0;
}
