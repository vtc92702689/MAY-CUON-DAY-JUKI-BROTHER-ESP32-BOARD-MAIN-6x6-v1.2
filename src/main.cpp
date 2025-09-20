#include <Arduino.h>
#include <OneButton.h>
#include "ota.h"
#include "func.h"  // Bao gồm file header func.h để sử dụng các hàm từ func.cpp


//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Khởi tạo đối tượng màn hình OLED U8G2
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Khởi tạo đối tượng màn hình OLED U8G2

// Thông tin mạng WiFi và OTA



StaticJsonDocument<200> jsonDoc;

const char* jsonString = R"()";
bool isChanged = false;
void tinhToanCaiDat();
void loadSetup();
void veGoc();
void xuatXungPWM(unsigned long thoiGianDao);

OneButton btnMenu(0, true,false);
OneButton btnSet(2, false,false);
OneButton btnUp(12, false,false);
OneButton btnDown(15, false,false);
OneButton btnRun(23,false,false);
OneButton btnEstop(13,false,false);


void btnMenuClick() {
  //Serial.println("Button Clicked (nhấn nhả)");
  if (displayScreen == "ScreenCD") {
    if (keyStr == "CD" && isChanged) {
      writeFile(jsonDoc,"/config.json");
    }
    showList(menuIndex);  // Hiển thị danh sách menu hiện tại
    displayScreen = "MENU";
  } else if (displayScreen == "ScreenEdit") {
    loadJsonSettings();
    displayScreen = "ScreenCD";
  } else if (displayScreen == "index" && mainStep == 0) {
    trangThaiHoatDong = 0;
    showList(menuIndex);  // Hiển thị danh sách menu hiện tại
    displayScreen = "MENU";
  } else if (displayScreen == "MENU" && mainStep == 0){
    displayScreen= "index";
    trangThaiHoatDong = 1;
    veGoc();
    //showText("HELLO", "ESP32-OPTION");
  } else if (displayScreen == "testIO"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "testOutput"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "screenTestMode" && testModeStep == 0){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "OTA"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnMenuLongPressStart() {
  if (displayScreen == "OTA") {
  }
}
// Hàm callback khi nút đang được giữ
void btnMenuDuringLongPress() {
  //Serial.println("Button is being Long Pressed (BtnMenu)");
}

void btnSetClick() {
  if (displayScreen == "MENU") {
    pIndex = 1;
    loadJsonSettings(); // Hiển thị giá trị thiết lập
    displayScreen = "ScreenCD"; // Chuyển màn hình sau khi xử lý dữ liệu thành công
  } else if (displayScreen == "ScreenCD" && editAllowed){
    if (keyStr == "CD"){
      columnIndex = maxLength-1;
      showEdit(columnIndex);
      displayScreen = "ScreenEdit";
    } else if (keyStr == "CN") {
      if (setupCodeStr == "CN1"){
        trangThaiHoatDong = 201;   //Trạng thái hoạt động 201 là trạng thái TestMode
        testModeStep = 0;
        chayTestMode = true;
        showText("TEST MODE", String("Step " + String(testModeStep)).c_str());
        displayScreen = "screenTestMode";
      } else if (setupCodeStr == "CN2"){
        trangThaiHoatDong = 202;   //Trạng thái hoạt động 202 là trạng thái TEST IO INPUT
        showText("TEST I/O", "TEST I/O INPUT");
        displayScreen = "testIO";
      } else if ((setupCodeStr == "CN3")){
        trangThaiHoatDong = 203;  //Trạng thái hoạt động 203 là trạng thái TEST IO OUTPUT
        testOutputStep = 0;
        displayScreen = "testOutput";
        hienThiTestOutput = true;
      } else if ((setupCodeStr == "CN5")){
        setupOTA();
        displayScreen = "OTA";
        trangThaiHoatDong = 204;  //Trạng thái hoạt động 204 là trạng thái OTA UPDATE+0
      } else {
        columnIndex = maxLength - 1;
        showEdit(columnIndex);
        displayScreen = "ScreenEdit";
      }
    }
  } else if (displayScreen == "ScreenEdit" && editAllowed)  {
    if (keyStr == "CD"){
      if (columnIndex - 1 < 0){
        columnIndex = maxLength-1;
      } else {
        columnIndex --;
      }
      showEdit(columnIndex);
    }
  } else if (displayScreen == "testOutput"){
    daoTinHieuOutput = true;
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnSetLongPressStart() {
  if (displayScreen == "ScreenEdit"){
    if (keyStr == "CD"){
      jsonDoc["main"]["main" + String(menuIndex)]["children"][setupCodeStr]["configuredValue"] = currentValue;
      isChanged = true;
      log("Đã lưu giá trị:" + String(currentValue) + " vào thẻ " + keyStr + "/" + setupCodeStr);
      loadJsonSettings();
      loadSetup();
      tinhToanCaiDat();
      displayScreen = "ScreenCD";
    } else if (keyStr == "CN"){
      if (setupCodeStr == "CN4" && currentValue == 1){
        reSet();
        showText("RESET","Tắt máy khởi động lại!");
        trangThaiHoatDong = 200;  //Trạng thái hoạt động 200 là reset, không cho phép thao tác nào
        displayScreen = "RESET";
      }
    }
  }
}

// Hàm callback khi nút đang được giữ
void btnSetDuringLongPress() {
  //showSetup("Setup", "OFF", "Dang giu nut");
}

void btnUpClick() {
  if (displayScreen == "MENU") {
    if (menuIndex + 1 > 3) {
      menuIndex = 1;  // Khi chỉ số vượt quá giới hạn, quay lại đầu danh sách
    } else {
      menuIndex++;    // Tăng menuIndex lên 1
    }
    showList(menuIndex);  // Hiển thị danh sách với chỉ số mới
  } else if (displayScreen == "ScreenCD") {
    if (pIndex + 1 > totalChildren) {
      pIndex = 1;
    } else {
      pIndex ++;
    }
    loadJsonSettings(); // Hiển thị giá trị thiết lập
  } else if (displayScreen == "ScreenEdit") {
    if (keyStr == "CD"){
      editValue("addition");
      log("Value:" + valueStr);
    } else if (keyStr == "CN") {
      editValue("addition");
      log("Value:" + valueStr);
    }
  } else if (displayScreen == "testOutput"){
    if (testOutputStep == maxTestOutputStep){
      testOutputStep = 0;
      hienThiTestOutput = true;
    } else {
      testOutputStep ++;
      hienThiTestOutput = true;
    }
  } else if (displayScreen == "screenTestMode"){
    if (testModeStep < maxTestModeStep){
      testModeStep ++;
    } else {
      testModeStep = 0;
    }
    chayTestMode = true;
    showText("TEST MODE", String("Step " + String(testModeStep)).c_str());
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnUpLongPressStart() { 
  //Serial.println("Button Long Press Started (btnUp)");
}

// Hàm callback khi nút đang được giữ
void btnUpDuringLongPress() {
  //Serial.println("Button is being Long Pressed (btnUp)");
}

void btnDownClick() {
  if (displayScreen == "MENU") {
    if (menuIndex - 1 < 1) {
      menuIndex = 3;  // Khi chỉ số nhỏ hơn giới hạn, quay lại cuối danh sách
    } else {
      menuIndex--;    // Giảm menuIndex đi 1
    }
    showList(menuIndex);  // Hiển thị danh sách với chỉ số mới
  } else if (displayScreen == "ScreenCD"){
    if (pIndex - 1 < 1) {
      pIndex = totalChildren;
    } else {
      pIndex --;
    }
    loadJsonSettings(); // Hiển thị giá trị thiết lập
  } else if (displayScreen == "ScreenEdit"){
    if (keyStr == "CD"){
      editValue("subtraction");
      log("Value:" + valueStr);
    } else if (keyStr == "CN"){
      editValue("subtraction");
      log("Value:" + valueStr);
    }
  } else if (displayScreen == "testOutput"){
    if (testOutputStep == 0){
      testOutputStep = maxTestOutputStep;
      hienThiTestOutput = true;
    } else {
      testOutputStep --;
      hienThiTestOutput = true;
    }
  } else if (displayScreen == "screenTestMode"){
    if (testModeStep > 0){
      /*testModeStep --;
      chayTestMode = true;
      showText("TEST MODE", String("Step " + String(testModeStep)).c_str());*/
    }
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnDownLongPressStart() {
  //Serial.println("Button Long Press Started (btnDown)");
}

// Hàm callback khi nút đang được giữ
void btnDownDuringLongPress() {
  //Serial.println("Button is being Long Pressed (btnDown)");
}

//KHAI BÁO CHÂN IO Ở ĐÂY

const int sensorCilinderXp1 = 36;
const int sensorCilinderXp2 = 39;
const int sensorCilinderYp1 = 34;
const int sensorCilinderYp2 = 35;
const int sensorOrigin = 32;
const int sensorFoot = 33;

const int sensorActive = 25;



const int outRelayX = 4;
const int outRelayY = 16;
const int outRelayFoot = 17;
const int outRelayRun = 5;
const int pinDir = 27;
const int pinPWM = 14;



//KHAI BÁO THÔNG SỐ TRƯƠNG TRÌNH

/* Ví dụ: int thoiGianNhaDao = 200;
          int soDuMuiDauVao = 10;*/

unsigned long thoiDiemCuoiPWM = 0;
bool trangThaiPWM = false;

byte cheDoHoatDong = 1;
byte soVongCuon = 2;
bool chieuQuayDongCo = 1;
int tocDoQuay = 500 ;
int soXungMotor = 400 ;
int soXungChuKy = 200;
int soXungDoGoc = 2000;

// Thông số tính toán

int soXungCanChay = 1000;
int thoiGianDaoPWM = 50;

// Biến toàn cục
bool active = false;

bool trangThaiCuoiCungX1 = false;
bool trangThaiCuoiCungX2 = false;
bool trangThaiCuoiCungY1 = false;
bool trangThaiCuoiCungY2 = false;
bool trangThaiCuoiCungOrigin = false;
bool trangThaiCuoiCungBanDap = false;



unsigned long thoiDiemCuoiKichChanVit = 0;
unsigned long thoiDiemCuoiKichMayChay = 0;

int soXungDaChay = 0;
int soVongDaChay = 0;

int loaiMay = 0;
int timeDelayBuoc = 20;
unsigned long lastTimeOut = 0;

int timeOut = 10000;
int doTreBatDau = 0;

//TRƯƠNG TRÌNH NGƯỜI DÙNG LẬP TRÌNH


void testMode(){
  switch (testModeStep){
    case 0:
      if (chayTestMode){
        maxTestModeStep = 7;
        soXungDaChay = 0;
        soVongDaChay = 0;
        digitalWrite(outRelayX,HIGH);
        digitalWrite(outRelayY,HIGH);
        chayTestMode = false;
      }
      break;
    case 1:
      if(chayTestMode){
        switch (cheDoHoatDong){
        case 1:
          if (soXungDaChay < soXungCanChay){
            xuatXungPWM(thoiGianDaoPWM);
          } else if (soXungDaChay == soXungCanChay && digitalRead(sensorOrigin)){
            //mainStep ++;
          }
          break;
        case 2:
          if (soXungDaChay < soXungCanChay){
            xuatXungPWM(thoiGianDaoPWM);
          } else if (soXungDaChay == soXungCanChay){
            //mainStep ++;
          }
          break;
        case 3:
          if (soVongDaChay < soVongCuon){
            xuatXungPWM(thoiGianDaoPWM);
            bool trangThaiHienTaiOrigin = digitalRead(sensorOrigin);
            if (trangThaiCuoiCungOrigin != trangThaiHienTaiOrigin){
              if (trangThaiHienTaiOrigin){
                soVongDaChay ++;
              }
              trangThaiCuoiCungOrigin = trangThaiHienTaiOrigin;
            }
          } else {
            //mainStep ++;
          }
          break;
        default:
          showText("ERROR","Lỗi trương trình");
          break;
        }
      }
      break;
    case 2:
      if (chayTestMode){
        digitalWrite(outRelayX,HIGH);
        digitalWrite(outRelayY,LOW);
        digitalWrite(outRelayFoot,LOW);
        digitalWrite(outRelayRun,LOW);
        chayTestMode = false;
      }
      break;
    case 3:
      if (chayTestMode){
        digitalWrite(outRelayX,HIGH);
        digitalWrite(outRelayY,LOW);
        digitalWrite(outRelayFoot,HIGH);
        digitalWrite(outRelayRun,LOW);
        chayTestMode = false;
      }
      break;
    case 4:
      if (chayTestMode){
        digitalWrite(outRelayX,LOW);
        digitalWrite(outRelayY,LOW);
        digitalWrite(outRelayFoot,LOW);
        digitalWrite(outRelayRun,LOW);
        chayTestMode = false;
      }
      break;
    case 5:{
      bool trangThaiGocStep = digitalRead(sensorOrigin);
      if(!trangThaiGocStep){
        xuatXungPWM(thoiGianDaoPWM);
      } 
      break;
    }
    case 6:
      if (chayTestMode){
        digitalWrite(outRelayX,LOW);
        digitalWrite(outRelayY,HIGH);
        digitalWrite(outRelayFoot,LOW);
        digitalWrite(outRelayRun,HIGH);
        chayTestMode = false;
      }
      break;
    case 7:
      if (chayTestMode){
        digitalWrite(outRelayX,HIGH);
        digitalWrite(outRelayY,HIGH);
        digitalWrite(outRelayFoot,LOW);
        digitalWrite(outRelayRun,LOW);
        chayTestMode = false;
      }
      break;
    default:
      /* code */
      break;
    }
}

void testInput(){
  static bool trangthaiCuoiIO1;
  if (digitalRead(sensorCilinderXp1)!= trangthaiCuoiIO1){
    trangthaiCuoiIO1 = digitalRead(sensorCilinderXp1);
    showText("IO 36" , String(trangthaiCuoiIO1).c_str());
  }
  static bool trangthaiCuoiIO2;
  if (digitalRead(sensorCilinderXp2)!= trangthaiCuoiIO2){
    trangthaiCuoiIO2 = digitalRead(sensorCilinderXp2);
    showText("IO 39" , String(trangthaiCuoiIO2).c_str());
  }
  static bool trangthaiCuoiIO3;
  if (digitalRead(sensorCilinderYp1)!= trangthaiCuoiIO3){
    trangthaiCuoiIO3 = digitalRead(sensorCilinderYp1);
    showText("IO 34" , String(trangthaiCuoiIO3).c_str());
  }
  static bool trangthaiCuoiIO4;
  if (digitalRead(sensorCilinderYp2)!= trangthaiCuoiIO4){
    trangthaiCuoiIO4 = digitalRead(sensorCilinderYp2);
    showText("IO 35" , String(trangthaiCuoiIO4).c_str());
  }
  static bool trangthaiCuoiIO5;
  if (digitalRead(sensorOrigin)!= trangthaiCuoiIO5){
    trangthaiCuoiIO5 = digitalRead(sensorOrigin);
    showText("IO 32" , String(trangthaiCuoiIO5).c_str());
  }
  static bool trangthaiCuoiIO6;
  if (digitalRead(sensorFoot)!= trangthaiCuoiIO6){
    trangthaiCuoiIO6 = digitalRead(sensorFoot);
    showText("IO 33" , String(trangthaiCuoiIO6).c_str());
  }
  static bool trangthaiCuoiIO7;
  if (digitalRead(sensorActive)!= trangthaiCuoiIO7){
    trangthaiCuoiIO7 = digitalRead(sensorActive);
    showText("IO 25" , String(trangthaiCuoiIO7).c_str());
  }
}

void testOutput(){
  switch (testOutputStep){
    case 0:
      if (hienThiTestOutput){
        maxTestOutputStep = 5;
        bool tinHieuHienTai = digitalRead(outRelayX);
        showText("IO 4", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(outRelayX);
        digitalWrite(outRelayX,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 1:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(outRelayY);
        showText("IO 16", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(outRelayY);
        digitalWrite(outRelayY,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 2:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(outRelayFoot);
        showText("IO 17", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(outRelayFoot);
        digitalWrite(outRelayFoot,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 3:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(outRelayRun);
        showText("IO 5", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(outRelayRun);
        digitalWrite(outRelayRun,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 4:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(pinDir);
        showText("IO 18", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(pinDir);
        digitalWrite(pinDir,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
      case 5:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(pinPWM);
        showText("IO 19", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(pinPWM);
        digitalWrite(pinPWM,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;


    default:
      break;
  }
}


void tinhToanCaiDat(){
  /* Ví dụ:
  soXungCanChay = soXungMotor * soVongCuon;
  */
  soXungCanChay = soXungChuKy * soVongCuon;
  thoiGianDaoPWM = (1000000*30)/(tocDoQuay*soXungMotor);
  digitalWrite(pinDir,chieuQuayDongCo);
}

void loadSetup(){
  /* Ví dụ:
  cheDoHoatDong = jsonDoc["main"]["main1"]["children"]["CD1"]["configuredValue"];
  */
  cheDoHoatDong = jsonDoc["main"]["main1"]["children"]["CD1"]["configuredValue"];
  soVongCuon = jsonDoc["main"]["main1"]["children"]["CD2"]["configuredValue"];
  chieuQuayDongCo = jsonDoc["main"]["main1"]["children"]["CD3"]["configuredValue"];
  tocDoQuay = jsonDoc["main"]["main1"]["children"]["CD4"]["configuredValue"] ;
  soXungMotor = jsonDoc["main"]["main1"]["children"]["CD5"]["configuredValue"] ;
  soXungChuKy = jsonDoc["main"]["main1"]["children"]["CD6"]["configuredValue"] ;
  loaiMay = jsonDoc["main"]["main1"]["children"]["CD7"]["configuredValue"] ;
  timeDelayBuoc = jsonDoc["main"]["main1"]["children"]["CD8"]["configuredValue"] ;
  doTreBatDau = jsonDoc["main"]["main1"]["children"]["CD9"]["configuredValue"] ;
}

void veGoc(){
  if(!active) {
    if (loaiMay == 1){
        digitalWrite(outRelayRun,LOW);
      //showText("HELLO","ESP32 VE GOC");
      if(digitalRead(sensorFoot)){
        showText("START","Brother - OPNR");
        delay(1000);
        trangThaiHoatDong = 199;
      } else {
        showText("START","Brother - OPR");
        delay(1000);
        trangThaiHoatDong = 198;
      }
    } else if (loaiMay == 2){
      showText("START","Juki - Chờ sẵn sàng");
      delay(1000);
      trangThaiHoatDong = 198;
    }
  } else {
    showText("READY","Sẵn Sàng");
  }
}
  
void khoiDong(){
  trangThaiCuoiCungX1 = digitalRead(sensorCilinderXp1);
  trangThaiCuoiCungX2 = digitalRead(sensorCilinderXp2);
  trangThaiCuoiCungY1 = digitalRead(sensorCilinderYp1);
  trangThaiCuoiCungY2 = digitalRead(sensorCilinderYp1);
  trangThaiCuoiCungOrigin = digitalRead(sensorOrigin);
  trangThaiCuoiCungBanDap = digitalRead(sensorActive);
  delay(200);
  displayScreen = "index";
  showText("HELLO","Xin Chào");
  mainStep = 0;
  trangThaiHoatDong = 0;
  loadSetup();
  delay(200);
  tinhToanCaiDat();
  delay(100);
  veGoc();
}

void xuatXungPWM(unsigned long thoiGianDao){
  if (WaitMicros(thoiDiemCuoiPWM,thoiGianDao) && !trangThaiPWM){
    trangThaiPWM = !trangThaiPWM;
    digitalWrite(pinPWM,trangThaiPWM);
    thoiDiemCuoiPWM = micros();
  } else if (WaitMicros(thoiDiemCuoiPWM,thoiGianDao) && trangThaiPWM){
    trangThaiPWM = !trangThaiPWM;
    digitalWrite(pinPWM,trangThaiPWM);
    thoiDiemCuoiPWM = micros();
    soXungDaChay ++;
  }
}

void mainRun(){
  switch (mainStep){
  case 0:
    
    break;
  case 1:
    switch (cheDoHoatDong){
    case 1:
      if (soXungDaChay < soXungCanChay){
        xuatXungPWM(thoiGianDaoPWM);
      } else if (soXungDaChay >= soXungCanChay && digitalRead(sensorOrigin)){
        mainStep ++;
        delay(timeDelayBuoc);
      } else {
        trangThaiHoatDong = 200;
        showSetup("ERROR","E011","Lỗi động cơ, c.biến gốc");
      }
      break;
    case 2:
      if (soXungDaChay < soXungCanChay){
        xuatXungPWM(thoiGianDaoPWM);
      } else if (soXungDaChay == soXungCanChay){
        mainStep ++;
        delay(timeDelayBuoc);
      }
      break;
    case 3:
      if (soVongDaChay < soVongCuon){
        xuatXungPWM(thoiGianDaoPWM);
        bool trangThaiHienTaiOrigin = digitalRead(sensorOrigin);
        if (trangThaiCuoiCungOrigin != trangThaiHienTaiOrigin){
          if (trangThaiHienTaiOrigin){
            lastTimeOut = millis();
            soVongDaChay ++;
          }
          trangThaiCuoiCungOrigin = trangThaiHienTaiOrigin;
        }
      } else {
        mainStep ++;
        delay(timeDelayBuoc);
      }
      if (WaitMillis(lastTimeOut,3500)) {
          trangThaiHoatDong = 200;
          showSetup("ERROR","E011","Lỗi động cơ, c.biến gốc");
      }
      break;
    default:
      showText("ERROR","Lỗi trương trình");
      break;
    }
    break;
  case 2:
    if (digitalRead(sensorCilinderXp1)){
      digitalWrite(outRelayY,LOW);
      mainStep++;
      delay(timeDelayBuoc);
    }
    break;
  case 3:
    if (digitalRead(sensorCilinderYp2)){
      digitalWrite(outRelayFoot,HIGH);
      mainStep++;
      delay(timeDelayBuoc);
    }
    break;
  case 4:
    if (digitalRead(sensorFoot)){
      digitalWrite(outRelayFoot,LOW);
      digitalWrite(outRelayX,LOW);
      mainStep ++;
      delay(timeDelayBuoc);
    }
    break;
  case 5:
    if (digitalRead(sensorCilinderXp2)) {
      digitalWrite(outRelayRun,HIGH);
      digitalWrite(outRelayY,HIGH);
      mainStep ++;
      delay(timeDelayBuoc);
      lastTimeOut = millis();
    }
    break;
  case 6:{
    bool trangThaiGocStep = digitalRead(sensorOrigin);
    if(!trangThaiGocStep){
      xuatXungPWM(thoiGianDaoPWM);
    }
    if (digitalRead(sensorCilinderYp1) && trangThaiGocStep){
      digitalWrite(outRelayRun,LOW);
      digitalWrite(outRelayX,HIGH);
      mainStep++;
      delay(timeDelayBuoc);
    } else if (WaitMillis(lastTimeOut,3500)) {
      trangThaiHoatDong = 200;
      showSetup("ERROR","E008","Không tìm thấy gốc STEP");
    }
    break;
  }
  case 7:
    if (digitalRead(sensorCilinderXp1)){
      soXungDaChay = 0;
      soVongDaChay = 0;
      trangThaiHoatDong = 1;
      mainStep = 0;
    }
    break;
  default:
    break;
  }
}



void setup() {

  Serial.begin(115200);     // Khởi tạo Serial và màn hình

  u8g2.begin();  // Khởi tạo màn hình OLED
  u8g2.enableUTF8Print(); // Kích hoạt hỗ trợ UTF-8

  btnMenu.attachClick(btnMenuClick);
  btnMenu.attachLongPressStart(btnMenuLongPressStart);
  btnMenu.attachDuringLongPress(btnMenuDuringLongPress);

  btnSet.attachClick(btnSetClick);
  btnSet.attachLongPressStart(btnSetLongPressStart);
  btnSet.attachDuringLongPress(btnSetDuringLongPress);

  btnUp.attachClick(btnUpClick);
  btnUp.attachLongPressStart(btnUpLongPressStart);
  btnUp.attachDuringLongPress(btnUpDuringLongPress);

  btnDown.attachClick(btnDownClick);  
  btnDown.attachLongPressStart(btnDownLongPressStart);
  btnDown.attachDuringLongPress(btnDownDuringLongPress);

  btnMenu.setDebounceMs(btnSetDebounceMill);
  btnSet.setDebounceMs(btnSetDebounceMill);
  btnUp.setDebounceMs(btnSetDebounceMill);
  btnDown.setDebounceMs(btnSetDebounceMill);

  btnMenu.setPressMs(btnSetPressMill);
  btnSet.setPressMs(btnSetPressMill);
  btnUp.setPressMs(btnSetPressMill);
  btnDown.setPressMs(btnSetPressMill);

  pinMode(sensorCilinderXp1,INPUT);
  pinMode(sensorCilinderXp2,INPUT);
  pinMode(sensorCilinderYp1,INPUT);
  pinMode(sensorCilinderYp2,INPUT);
  pinMode(sensorOrigin,INPUT);
  pinMode(sensorFoot,INPUT);
  pinMode(sensorActive,INPUT);


  pinMode(outRelayX,OUTPUT);
  pinMode(outRelayY,OUTPUT);
  pinMode(outRelayFoot,OUTPUT);
  pinMode(outRelayRun,OUTPUT);
  pinMode(pinDir,OUTPUT);
  pinMode(pinPWM,OUTPUT);



  if (!LittleFS.begin()) {
    showSetup("Error", "E003", "LittleFS Mount Failed");
    Serial.println("LittleFS Mount Failed");
    return;
  }

  

  // Kiểm tra xem file config.json có tồn tại không
  if (!LittleFS.exists(configFile)) {
    DeserializationError error = deserializeJson(jsonDoc, jsonString); // Phân tích chuỗi JSON
    if (error) {
        showSetup("Error", "E005", "JsonString Error");
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    showSetup("Error", "E007", "JsonString Load Mode");
    Serial.println("Read Data From JsonString");
    loadSetup();
    Serial.println("File config.json does not exist.");
    return;
  }

  readConfigFile();

  Serial.println("Load toàn bộ dữ liệu thành công");
  khoiDong();
}

void loop() {
  switch (trangThaiHoatDong){
  case 0:
    btnMenu.tick();
    btnSet.tick();
    btnUp.tick();
    btnDown.tick();
    break;
  case 1:{
      btnMenu.tick();
      // Đảm bảo rằng sensorActive đã được định nghĩa đúng
      bool trangThaiBanDap = digitalRead(sensorActive);
      //showText(String(trangThaiBanDap).c_str(), String(trangThaiCuoiCungBanDap).c_str());
      if (trangThaiCuoiCungBanDap != trangThaiBanDap) {
        trangThaiCuoiCungBanDap = trangThaiBanDap;
        //showText(String(trangThaiBanDap).c_str(), String(trangThaiCuoiCungBanDap).c_str());
        if (!trangThaiBanDap && mainStep == 0) {
          lastTimeOut = millis();
          mainStep ++;
          trangThaiHoatDong = 2;
        }
      }
      break;
  }
  case 2:
    mainRun();
    break;
  case 198:{
      static int buocVeGoc = 0;
      switch (buocVeGoc){
        case 0:
          if (!digitalRead(sensorFoot)){
            buocVeGoc++;
          }
          break;
        case 1:
          if(!digitalRead(sensorOrigin)){
            xuatXungPWM(thoiGianDaoPWM);
          } else {
            showText("RUNNING","ĐANG VỀ GỐC");
            trangThaiHoatDong = 198;
            soXungDaChay = 0;
            lastTimeOut = millis();
            trangThaiCuoiCungOrigin = true;   //Cắm cờ tránh lỗi lần đầu chạy
            buocVeGoc ++;
          }
          if (soXungDaChay > soXungDoGoc){
            trangThaiHoatDong = 200;
            showSetup("ERROR","E008","Không tìm thấy gốc STEP");
          }
          break;
        case 2:
            if (digitalRead(sensorCilinderXp2)){
              digitalWrite(outRelayY,HIGH);
              lastTimeOut = millis();
              buocVeGoc++;
            } else {
              if(WaitMillis(lastTimeOut,timeOut)){
                trangThaiHoatDong = 200;
                showSetup("ERROR","E010","Kiểm tra cylinder X");
              }
            }
            break;
        case 3:
          if (digitalRead(sensorCilinderYp1)){
            digitalWrite(outRelayX,HIGH);
            lastTimeOut = millis();
            buocVeGoc++;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
              trangThaiHoatDong = 200;
              showSetup("ERROR","E011","Kiểm tra cylinder Y");
            }
          }
          break;
        case 4:
          if (digitalRead(sensorCilinderXp1)){
            active = true;
            showText("READY","Sẵn Sàng");
            trangThaiHoatDong = 1;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
            trangThaiHoatDong = 200;
              showSetup("ERROR","E010","Kiểm tra cylinder X");
            }
          }
          break;
      }
      break;
    }
    case 199:{
      static int buocVeGoc = 0;
      switch (buocVeGoc){
        case 0:
          if(!digitalRead(sensorOrigin)){
            xuatXungPWM(thoiGianDaoPWM);
          } else {
            showText("RUNNING","ĐANG VỀ GỐC");
            soXungDaChay = 0;
            lastTimeOut = millis();
            trangThaiCuoiCungOrigin = true;   //Cắm cờ tránh lỗi lần đầu chạy
            buocVeGoc ++;
          }
          if (soXungDaChay > soXungDoGoc){
            trangThaiHoatDong = 200;
            showSetup("ERROR","E008","Không tìm thấy gốc STEP");
          }
          break;
        case 1:
          if(WaitMillis(lastTimeOut,doTreBatDau)){
            if (digitalRead(sensorFoot)){
              digitalWrite(outRelayRun,HIGH);
              buocVeGoc ++;
            } else {
              trangThaiHoatDong = 200;
              showSetup("ERROR","E009","K.Tra cảm biến chân vịt");
            }
            lastTimeOut = millis();
          }
          break;
        case 2:
          if (!digitalRead(sensorFoot)){
            digitalWrite(outRelayRun,LOW);
            lastTimeOut = millis();
            buocVeGoc ++;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
              trangThaiHoatDong = 200;
              showSetup("ERROR","E009","K.Tra cảm biến chân vịt");
            }
          }
          break;
        case 3:
          if (digitalRead(sensorCilinderXp2)){
            digitalWrite(outRelayY,HIGH);
            lastTimeOut = millis();
            buocVeGoc++;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
              trangThaiHoatDong = 200;
              showSetup("ERROR","E010","Kiểm tra cylinder X");
            }
          }
          break;
        case 4:
          if (digitalRead(sensorCilinderYp1)){
            digitalWrite(outRelayX,HIGH);
            lastTimeOut = millis();
            buocVeGoc++;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
              trangThaiHoatDong = 200;
              showSetup("ERROR","E011","Kiểm tra cylinder Y");
            }
          }
          break;
        case 5:
          if (digitalRead(sensorCilinderXp1)){
            showText("READY","Sẵn sàng");
            active = true;
            trangThaiHoatDong = 1;
          } else {
            if(WaitMillis(lastTimeOut,timeOut)){
              trangThaiHoatDong = 200;
              showSetup("ERROR","E010","Kiểm tra cylinder X");
            }
          }
          break;
        }
      break;
    }
  case 200:        //ESTOP dừng khẩn cấp
    btnMenu.tick();
    break;
  case 201:         // Func Test Mode
    btnMenu.tick();
    btnUp.tick();
    btnDown.tick();
    testMode();
    break;
  case 202:        // Func Test Input
    btnMenu.tick();
    testInput();
    break;
  case 203:      // Func Test Output
    btnMenu.tick();
    btnSet.tick();
    btnUp.tick();
    btnDown.tick();
    testOutput();
    break;
  case 204:
    btnMenu.tick();
    handleOTA(); // Xử lý OTA khi điều kiện đúng
    break;  
  default:
    break;
  }
}
