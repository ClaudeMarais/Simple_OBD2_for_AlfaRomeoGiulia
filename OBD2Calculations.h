#ifndef _OBD2_CALCULATIONS
#define _OBD2_CALCULATIONS

// --------------------------------------------------------
// ******** Engine RPM ************************************
// --------------------------------------------------------

static int32_t g_EngineRPM = 0;

int32_t CalcEngineRPM(const uint8_t* pData)
{
  uint8_t A = pData[4];
  uint8_t B = pData[5];
  g_EngineRPM = ((int32_t(A) * 256) + int32_t(B)) / 4;
  return g_EngineRPM;
}

void PrintEngineRPM()
{
  Serial.printf("Engine RPM = %d\n", g_EngineRPM);
}

// --------------------------------------------------------
// ******** Currently Engaged Gear ************************
// --------------------------------------------------------

static int32_t g_Gear = 0;    // 0 = Neutral, -1 = Reverse

int32_t CalcGear(const uint8_t* pData)
{
  const uint8_t neutral = 0;
  const uint8_t reverse = 0x10;

  uint8_t A = pData[4];
  g_Gear = A;

  // Return -1 for reverse
  if (g_Gear == reverse)
  {
    g_Gear = -1;
  }

  return g_Gear;
}

void PrintGear()
{
  char gearStr[32] = "Neutral";

  if (g_Gear == -1)
  {
    sprintf(gearStr, "Reverse");
  }
  else if (g_Gear > 0)
  {
    sprintf(gearStr, "%d", g_Gear);
  }

  Serial.printf("Current Engaged Gear = %s\n", gearStr);
}

// --------------------------------------------------------
// ******** Engine Oil Temperature ************************
// --------------------------------------------------------

static int32_t g_EngineOilTemp = 0;   // Celcius

int32_t CalcEngineOilTemp(const uint8_t* pData)
{
  uint8_t B = pData[5];
  g_EngineOilTemp = B;
  return g_EngineOilTemp;
}

void PrintEngineOilTemp()
{
  int32_t Farh = (float(g_EngineOilTemp) * 9.0f / 5.0f) + 32.0f + 0.5f;
  Serial.printf("Engine Oil Temperature = %d C (%d F)\n", g_EngineOilTemp, Farh);
}

// --------------------------------------------------------
// ******** Battery IBS ***********************************
// --------------------------------------------------------

static int32_t g_BatteryIBS = 0;    // %

int32_t CalcBatteryIBS(const uint8_t* pData)
{
  uint8_t A = pData[4];
  g_BatteryIBS = A;
  return g_BatteryIBS;
}

void PrintBatteryIBS()
{
  Serial.printf("Battery IBS = %d %%\n", g_BatteryIBS);
}

// --------------------------------------------------------
// ******** Battery ***************************************
// --------------------------------------------------------

static float g_Battery = 0;     // Volts

int32_t CalcBattery(const uint8_t* pData)
{
  uint8_t B = pData[5];
  g_Battery = B / 10.0f;
  return g_Battery;
}

void PrintBattery()
{
  Serial.printf("Battery = %.1f Volts\n", g_Battery);
}

// --------------------------------------------------------
// ******** Atmospheric Pressure***************************
// --------------------------------------------------------

static int32_t g_AtmosphericPressure = 0;   // mbar

int32_t CalcAtmosphericPressure(const uint8_t* pData)
{
  uint8_t A = pData[4];
  uint8_t B = pData[5];
  g_AtmosphericPressure = (A * 256 + B);
  return g_AtmosphericPressure;
}

void PrintAtmosphericPressure()
{
  Serial.printf("Atmospheric Pressure = %d mbar\n", g_AtmosphericPressure);
}

// --------------------------------------------------------
// ******** Boost Pressure ********************************
// --------------------------------------------------------

static int32_t g_BoostPressure = 0;  // mbar

int32_t CalcBoostPressure(const uint8_t* pData)
{
  uint8_t A = pData[4];
  uint8_t B = pData[5];
  g_BoostPressure = (A * 256 + B);
  return g_BoostPressure;
}

void PrintBoostPressure()
{
  Serial.printf("Boost Pressure = %d mbar\n", g_BoostPressure);
}

// --------------------------------------------------------
// ******** External Temperature **************************
// --------------------------------------------------------

static int32_t g_ExternalTemp = 0;    // Celcius

int32_t CalcExternalTemp(const uint8_t* pData)
{
  uint8_t A = pData[4];
  g_ExternalTemp = (A / 2) - 40;
  return g_ExternalTemp;
}

void PrintExternalTemp()
{
  int32_t Farh = (float(g_ExternalTemp) * 9.0f / 5.0f) + 32.0f + 0.5f;
  Serial.printf("External Temperature = %d C (%d F)\n", g_ExternalTemp, Farh);
}

#endif
