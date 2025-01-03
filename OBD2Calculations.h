// --------------------------------------------------------
// ******** Engine RPM ************************************
// --------------------------------------------------------

static uint32_t g_EngineRPM = 0;

uint32_t CalcEngineRPM(const uint8_t* pData)
{
  uint8_t A = pData[4];
  uint8_t B = pData[5];
  g_EngineRPM = ((uint32_t(A) * 256) + uint32_t(B)) / 4;
  return g_EngineRPM;
}

void PrintEngineRPM()
{
  Serial.printf("Engine RPM = %d\n", g_EngineRPM);
}

// --------------------------------------------------------
// ******** Engine Oil Temperature ************************
// --------------------------------------------------------

static uint32_t g_EngineOilTemp = 0;    // In Celcius

uint32_t CalcEngineOilTemp(const uint8_t* pData)
{
  uint8_t B = pData[5];
  g_EngineOilTemp = B;
  return g_EngineOilTemp;
}

void PrintEngineOilTemp()
{
  uint32_t Farh = (float(g_EngineOilTemp) * 9.0f / 5.0f) + 32.0f + 0.5f;
  Serial.printf("Engine Oil Temperature = %d C (%d F)\n", g_EngineOilTemp, Farh);
}
