#include "raylib.h"
#include "Managers.h"
#include "UI.h"

int main() {
    // 1. Khởi tạo và nạp dữ liệu từ các file CSV
    UserManager userManager;
    StationManager stationManager;
    SessionManager sessionManager;

    DataManager::loadAll(userManager, stationManager, sessionManager);

    // 2. Khởi tạo cửa sổ đồ họa
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "EV Charging Station Management");
    SetTargetFPS(60);

    AppUI appUI(userManager, stationManager, sessionManager);
    appUI.Init();

    // 3. Vòng lặp chính của chương trình
    while (!WindowShouldClose()) {
        appUI.UpdateDrawFrame();
    }

    // 4. Tự động lưu toàn bộ dữ liệu trước khi thoát
    DataManager::saveAll(userManager, stationManager, sessionManager);

    CloseWindow();
    return 0;
}
