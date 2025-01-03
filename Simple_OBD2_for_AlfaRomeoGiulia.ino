// A simple Arduino project to show how to communicate with your car's OBD2 port using an ESP32-C3 and SN65HVD230 CAN bus transceiver, using ESP32 TWAI (Two-Wire Automotive Interface)
//
// NOTE: It's fun to tinker with your car, but there is always a chance to mess things up. I won't be liable if for some reason you damage your car.
//
// NOTE: The CAN IDs and PIDs used in this app specifically works with a 2019 Alfa Romeo Giulia 2.0L (Petrol).
//       It's highly unlikely that the same PIDs will work with another car, you'll have to research what PIDs work with your own car.
//
// A big thank you to the Alfisti community for reverse enginering some of these PIDs!
//
// Some tips:
//
// 1) Connect your car to a battery charger while experimenting. It's highly likely that you'll spend several hours in your car while the battery is being drained.
// 2) Diagrams of OBD2 pins are normally shown for the female connector in your car. Don't forget that those pins are in swapped/mirrored positions on the male connector.
// 3) The OBD2 connector has an "always on" 12V pin. Make sure the wire connecting to that pin on your male connector isn't exposed so that it cannot touch other wires!
// 4) I tried multiple pins on the ESP32-C3 to connect to the SN65HVD230, but only D4/D5 worked for me. Coincidentally these are also the SDA/SCL pins.
// 5) Check if your car has an OBD2 Security Gateway (SGW). If so, you need to install a SGW Bypass module before you to send/receive OBD2 frames to your car.

// Hardware:
//    XIAO ESP32-C3
//    SN65HVD230 CAN bus tranceiver
//
// Arduino library used:
//    ESP32-TWAI-CAN found here: https://github.com/handmade0octopus/ESP32-TWAI-CAN
//

// The following information is very helpful to understand how OBD2 uses the CAN bus for communication:
//    https://www.csselectronics.com/pages/obd2-explained-simple-intro
//
// General CAN Frame (CAN frames can be any size)
//   _________________________________________________________________
//  |            |       |         |       |        |        |        |
//  |  CAN ID    |  DLC  |  Mode   |  PID  |  Data  |  Data  | ...    |
//  |____________|_______|_________|_______|________|________|________|
//
// OBD2 Frame with 2-byte PIDs (OBD2 frames are always 8 bytes)
//   __________________________________________________________________________________
//  |            |       |         |       |       |        |        |        |        |
//  | Car Module |  DLC  | Service |  PID  |  PID  |  Data  |  Data  |  Data  |  Data  |
//  |____________|_______|_________|_______|_______|________|________|________|________|
//  

#include <ESP32-TWAI-CAN.hpp>   // TWAI = Two-Wire Automotive Interface
#include "OBD2Calculations.h"   // Callback functions for PIDs

// CAN IDs of car modules
enum CarModule
{
  All = 0x18DB33F1,   // Used to send a message to all car modules
  ECM = 0x18DA10F1,   // Engine Control Module
  TCM = 0x18DA18F1    // Transmision Control Module
};

// CAN Modes for OBD2 Services
enum OBD2Service
{
  CurrentData           = 0x01,
  TroubleCodes          = 0x03,
  VehicleInfo           = 0x09,
  ManufacturerSpecific  = 0x22
};

// Struct to easily define PIDs
struct PID
{
    char        Name[256];
    CarModule   Module;
    OBD2Service Service;
    uint16_t    PID;
    uint32_t (*CalculateValue)(const uint8_t* pData);
    void (*PrintInformation)(void);
};

// Define our OBD2 PIDs (Thanks to the Alfisti community for reverse enginering some of these PIDs)
PID PIDs[] = { { "Engine RPM",      CarModule::ECM, OBD2Service::ManufacturerSpecific, 0x1000, &CalcEngineRPM,     PrintEngineRPM },
               { "Engine Oil Temp", CarModule::ECM, OBD2Service::ManufacturerSpecific, 0x1302, &CalcEngineOilTemp, PrintEngineOilTemp} };

// Index into the above PIDs[] declaration
enum PIDIndex
{
  EngineRPM,
  EngineOilTemp,
  NumPIDs
};

PID* pEngineRPM      = &PIDs[PIDIndex::EngineRPM];
PID* pEngineOilTemp  = &PIDs[PIDIndex::EngineOilTemp];

// Most of the PIDs for this car are two bytes and sometimes we need to work with one byte at a time
#define FIRST_BYTE(TwoByteNumber)   (TwoByteNumber >> 8)
#define SECOND_BYTE(TwoByteNumber)  (TwoByteNumber & 0x00FF)

// We'll receive OBD2 data in this CAN frame
CanFrame ReceivedOBD2Frame;

// Send a request for OBD2 data
void SendOBD2Request(CarModule carModule, OBD2Service service, uint16_t pid)
{
  const uint8_t   unused = 0xAA;

	CanFrame canFrame = { 0 };

	canFrame.identifier = carModule;
	canFrame.extd = (carModule > 0xFF);       // Standard CAN IDs are in the range 0x7E8-0x7EF
	canFrame.data_length_code = 8;            // OBD2 always has 8 bytes in a CAN frame
	canFrame.data[0] = (pid > 0xFF) ? 3 : 2;  // If pid is 1 byte, then payload is 2 (1 byte for DLC and 1 byte for pid), otherwise payload is 3 (1 byte for DLC and 2 bytes for pid)
	canFrame.data[1] = service;
	canFrame.data[2] = (pid > 0xFF) ? FIRST_BYTE(pid) : pid;      // If the pid is 2 bytes, use the most signigicant byte as the 1st byte in the data
	canFrame.data[3] = (pid > 0xFF) ? SECOND_BYTE(pid) : unused;  // If the pid is 2 bytes, use the least signigicant byte as the 2nd byte in the data
	canFrame.data[4] = unused;
	canFrame.data[5] = unused;
	canFrame.data[6] = unused;
	canFrame.data[7] = unused;

  ESP32Can.writeFrame(canFrame);

  delay(50);  // Add a short delay between sending frames
}

// Find the PID in the data of an OBD2 frame. Most of our PIDs will be two bytes long
uint16_t GetPID(const CanFrame& frame, uint8_t pidLengthInBytes = 2)
{
  return (pidLengthInBytes == 1) ? frame.data[2] : ((frame.data[2] << 8) | frame.data[3]);
}

// Determine if a CAN ID is from a valid OBD2 car module. Valid CAN IDs for OBD2 are in the range [0x18DAF100 .. 0x18DAF1FF]
// To be safe, we'll check the range [0x18000000 .. 0x18FFFFFF]
bool IsValidCarModule(uint32_t canID)
{
  return (canID >= 0x18000000) && (canID <= 0x18FFFFFF);
}

// It's useful for debugging to print the raw data of an OBD2 frame
void PrintOBD2Frame(CanFrame& frame)
{
  // Print module and PID
  auto pid = GetPID(frame);
  Serial.printf("%#08x %#04x : ", frame.identifier, pid);
  
  // Print data
  for (int i = 0; i < frame.data_length_code; i++)
  {
    Serial.printf("%#02x ", frame.data[i]);
  }

  Serial.println();      
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  
  Serial.printf("\n\n********* Simple OBD2 for Alfa Romeo Giulia using ESP32-C3 *********\n\n");

  // Random frames with non-valid OBD2 data can be received on the CAN bus. We're not interested in those, therefore we want to filter them out.
  // Valid extended CAN IDs are in the range [0x18DAF100 .. 0x18DAF1FF], so we setup a filter to only receive CAN IDs in the range [0x18000000 .. 0x18FFFFFF]
  twai_filter_config_t canFilter;
	canFilter.acceptance_code = 0x18000000U << 3;
	canFilter.acceptance_mask = 0x00FFFFFFU << 3;
	canFilter.single_filter = true;

  // Initialize TWAI (two-wire automotive interface) for CAN messaging
  // NOTE: After some failed attempts with the ESP32-C3, it seems that pins TX/RX and D0/D1 doesn't send/receive data to/from
  // SN65HVD230 correctly, but D4/D5 does. (which coinsidentally is also SDA/SCL)
  while (!ESP32Can.begin(TWAI_SPEED_500KBPS, D4, D5, 8, 8, &canFilter))
  {
    Serial.println("CAN bus failed!");
    delay(1000);
  }

  Serial.println("CAN bus started!");
}

void loop()
{
  static uint32_t lastTimeStamp = 0;
  uint32_t currentTimeStamp = millis();

  const int timeIntervalForData = 250;  // Get data every 250 ms

  // Send OBD2 requests
  if (currentTimeStamp - lastTimeStamp > timeIntervalForData)
  {
    lastTimeStamp = currentTimeStamp;

    SendOBD2Request(pEngineRPM->Module, pEngineRPM->Service, pEngineRPM->PID);
    SendOBD2Request(pEngineOilTemp->Module, pEngineOilTemp->Service, pEngineOilTemp->PID);
  }

  // Listen for OBD2 frames that came back after requests were sent to retrieve data
  while (ESP32Can.readFrame(ReceivedOBD2Frame, timeIntervalForData))
  {
    auto canID = ReceivedOBD2Frame.identifier;
    auto pid = GetPID(ReceivedOBD2Frame);

    if (IsValidCarModule(canID))
    {
      for (int i = 0; i < NumPIDs; i++)
      {
        if (pid == PIDs[i].PID)
        {
          PIDs[i].CalculateValue(ReceivedOBD2Frame.data);
          PIDs[i].PrintInformation();
        }
      }
    }
    else
    {
      // Using the TWAI filter/mask, we shouldn't see any invalid OBD2 messages
      Serial.printf("Invalid OBD2 frame\n");
      PrintOBD2Frame(ReceivedOBD2Frame);
    }
  }

}