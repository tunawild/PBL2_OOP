#pragma once
#include <string>
#include <vector>

using namespace std;

// =====================================================================
// ENUM DINH NGHIA CAC KIEU DU LIEU CO SO
// =====================================================================

/**
 * @brief Enum phan quyen nguoi dung trong he thong
 */
enum UserRole {
    ROLE_ADMIN,     // Quan tri vien he thong
    ROLE_CUSTOMER   // Khach hang su dung tram sac
};

/**
 * @brief Enum phan loai cong sac
 */
enum PortType {
    AC_STANDARD = 0,    // Sac tieu chuan AC (Cham/Trung binh, ~11-22kW)
    DC_FAST = 1,        // Sac nhanh DC (~50-60kW)
    DC_SUPERCHARGE = 2  // Sac sieu nhanh DC Supercharge (~150-250kW)
};

/**
 * @brief Enum trang thai hoat dong cua cong sac
 */
enum PortStatus {
    AVAILABLE = 0,      // Cong trong, san sang phuc vu
    OCCUPIED = 1,       // Cong dang duoc xe ket noi va sac
    MAINTENANCE = 2     // Cong dang bao tri hoac gap su co ky thuat
};

// =====================================================================
// LOP CO SO NGUOI DUNG (OOP: BASE CLASS & ABSTRACTION)
// =====================================================================

/**
 * @brief Lop co so truu tuong User dai dien cho nguoi dung he thong.
 * The hien tinh Dong goi (Encapsulation) qua cac thuoc tinh protected/private
 * va tinh Da hinh (Polymorphism) qua ham thuan ao getRoleName().
 */
class User {
protected:
    string id;          // Ma dinh danh nguoi dung (PK)
    string username;    // Ten dang nhap duy nhat
    string password;    // Mat khau tai khoan
    UserRole role;      // Vai tro trong he thong

public:
    /**
     * @brief Constructor khoi tao thong tin nguoi dung
     */
    User(const string& id, const string& user, const string& pass, UserRole r);

    /**
     * @brief Destructor ao dam bao huy dung doi tuong con (Polymorphic destruction)
     */
    virtual ~User() = default;

    // --- Cac phuong thuc Getter (Tinh Dong goi) ---
    string getId() const;
    string getUsername() const;
    string getPassword() const;
    UserRole getRole() const;

    /**
     * @brief Kiem tra tinh hop le cua mat khau dang nhap
     * @param pass Mat khau can kiem tra
     * @return true neu khop, nguoc lai false
     */
    bool checkPassword(const string& pass) const;

    /**
     * @brief Phuong thuc thuan ao lay ten vai tro (Tinh Da hinh)
     */
    virtual string getRoleName() const = 0;
};

// =====================================================================
// LOP ADMIN (OOP: INHERITANCE)
// =====================================================================

/**
 * @brief Lop Admin ke thua tu lop User, quan ly toan bo he thong tram sac
 */
class Admin : public User {
public:
    /**
     * @brief Constructor khoi tao Admin voi role mac dinh la ROLE_ADMIN
     */
    Admin(const string& id, const string& user, const string& pass);

    /**
     * @brief Ghi de phuong thuc da hinh de tra ve ten vai tro Admin
     */
    virtual string getRoleName() const override;
};

// =====================================================================
// LOP CUSTOMER (OOP: INHERITANCE & ENCAPSULATION)
// =====================================================================

/**
 * @brief Lop Customer ke thua tu lop User, dai dien cho khach hang sac xe.
 * Bao gom cac thong tin xe dien, dung luong pin xe va so du vi.
 * (Khong luu tru % pin ma chi luu dung luong pin thuc te kWh).
 */
class Customer : public User {
private:
    string name;            // Ho va ten khach hang
    string phone;           // So dien thoai lien he
    string licensePlate;    // Bien so xe dang ky
    string carModel;        // Dong xe (vi du: VF8, Taycan...)
    float batteryCapacity;  // Dung luong pin cua xe (kWh)
    float balance;          // So du tai khoan vi dien tu (VND)

public:
    /**
     * @brief Constructor khoi tao khach hang day du thong tin
     */
    Customer(const string& id, const string& user, const string& pass,
             const string& name, const string& phone, const string& license,
             const string& carModel, float capacity, float bal);

    // --- Cac ham Getter ---
    string getName() const;
    string getPhone() const;
    string getLicensePlate() const;
    string getCarModel() const;
    float getBatteryCapacity() const;
    float getBalance() const;

    // --- Cac ham Setter / Mutator nghiep vu ---
    /**
     * @brief Tru tien khoi so du vi khi thanh toan phien sac
     * @param amount So tien can tru
     */
    void deductBalance(float amount);

    /**
     * @brief Cong tien vao so du vi (do Admin thuc hien nap tien)
     * @param amount So tien can nap
     */
    void addBalance(float amount);

    /**
     * @brief Ghi de phuong thuc da hinh tra ve ten vai tro Customer
     */
    virtual string getRoleName() const override;
};

// =====================================================================
// LOP CHARGING PORT (CONG SAC)
// =====================================================================

/**
 * @brief Lop ChargingPort dai dien cho mot tru/cong sac thuoc tram sac.
 * Dong goi day du thuoc tinh va cung cap getter/setter.
 */
class ChargingPort {
private:
    string portId;              // Ma cong sac trong tram (vi du: P01, P02)
    PortType type;              // Loai cong sac (AC, DC Fast, DC Supercharge)
    float powerKW;              // Cong suat toi da cua cong (kW)
    PortStatus status;          // Trang thai hien tai (Available, Occupied, Maintenance)
    float pricePerKWh;          // Don gia dien tai cong (VND/kWh)
    string currentSessionId;    // Ma phien sac dang thuc hien (neu dang sac)

public:
    /**
     * @brief Constructor khoi tao cong sac
     */
    ChargingPort(const string& id, PortType type, float pwr, float price,
                 PortStatus st = AVAILABLE, const string& sessId = "");

    // --- Getters ---
    string getPortId() const;
    PortType getType() const;
    float getPowerKW() const;
    PortStatus getStatus() const;
    float getPricePerKWh() const;
    string getCurrentSessionId() const;

    // --- Setters ---
    void setStatus(PortStatus newStatus);
    void setPricePerKWh(float newPrice);
    void setCurrentSessionId(const string& sessId);
};

// =====================================================================
// LOP CHARGING STATION (TRAM SAC - OOP: COMPOSITION/AGGREGATION)
// =====================================================================

/**
 * @brief Lop ChargingStation dai dien cho mot tram sac xe dien,
 * quan ly mot tap hop cac cong sac (Composition voi ChargingPort).
 */
class ChargingStation {
private:
    string stationId;               // Ma tram sac (vi du: ST01, ST02)
    string name;                    // Ten tram sac
    string address;                 // Dia chi dat tram sac
    vector<ChargingPort> ports;     // Danh sach cac cong sac tai tram

public:
    /**
     * @brief Constructor khoi tao tram sac
     */
    ChargingStation(const string& id, const string& n, const string& addr);

    // --- Getters ---
    string getStationId() const;
    string getName() const;
    string getAddress() const;
    const vector<ChargingPort>& getPorts() const;
    vector<ChargingPort>& getPorts(); // Non-const getter cho phep sua cong

    // --- Setters ---
    void setName(const string& n);
    void setAddress(const string& addr);

    // --- Quan ly danh sach cong sac ---
    /**
     * @brief Them mot cong sac moi vao tram
     */
    void addPort(const ChargingPort& port);

    /**
     * @brief Xoa mot cong sac khoi tram theo portId
     * @return true neu xoa thanh cong, false neu khong tim thay
     */
    bool removePort(const string& portId);

    /**
     * @brief Tim con tro den cong sac theo portId
     */
    ChargingPort* getPortById(const string& portId);
};

// =====================================================================
// LOP CHARGING SESSION (PHIEN SAC)
// =====================================================================

/**
 * @brief Lop ChargingSession ghi lai toan bo thong tin mot phien sac xe.
 */
class ChargingSession {
private:
    string sessionId;       // Ma phien sac duy nhat (PK)
    string customerId;      // Ma khach hang thuc hien sac (FK)
    string stationId;       // Ma tram sac (FK)
    string portId;          // Ma cong sac (FK)
    long long startTime;    // Thoi gian bat dau sac (timestamp)
    long long endTime;      // Thoi gian ket thuc sac (timestamp)
    float energyKWh;        // Tong dien nang tieu thu trong phien (kWh)
    float cost;             // Tong chi phi phien sac (VND)
    bool isActive;          // Trang thai phien sac (true: dang dien ra, false: da ket thuc)

public:
    /**
     * @brief Constructor khoi tao phien sac moi
     */
    ChargingSession(const string& sId, const string& cId, const string& stId,
                    const string& pId, long long start);

    // --- Getters ---
    string getSessionId() const;
    string getCustomerId() const;
    string getStationId() const;
    string getPortId() const;
    long long getStartTime() const;
    long long getEndTime() const;
    float getEnergyKWh() const;
    float getCost() const;
    bool getIsActive() const;

    // --- Setters nghiep vu ---
    void setEndTime(long long endT);
    void setEnergyKWh(float energy);
    void setCost(float c);
    void setIsActive(bool active);

    /**
     * @brief Ket thuc phien sac, ghi nhan thoi gian va tinh tong tien
     */
    void endSession(long long endT, float energy, float totalCost);
};

// =====================================================================
// LOP INVOICE (HOA DON THANH TOAN)
// =====================================================================

/**
 * @brief Lop Invoice dai dien cho hoa don dien tu sau khi ket thuc phien sac.
 */
class Invoice {
private:
    string invoiceId;   // Ma hoa don duy nhat (PK)
    string sessionId;   // Ma phien sac tuong ung (FK)
    string customerId;  // Ma khach hang thanh toan (FK)
    float totalAmount;  // Tong so tien thanh toan (VND)
    long long date;     // Ngay gio lap hoa don (timestamp)

public:
    /**
     * @brief Constructor khoi tao hoa don
     */
    Invoice(const string& id, const string& sId, const string& cId, float amount, long long d);

    // --- Getters ---
    string getInvoiceId() const;
    string getSessionId() const;
    string getCustomerId() const;
    float getTotalAmount() const;
    long long getDate() const;
};
