#include "vpd.h"
#include <math.h>

float vpdSvp(float tempC) {
  return 0.6108f * expf(17.27f * tempC / (tempC + 237.3f));
}

static float clampRh(float rh) {
  if (rh < 0.0f) return 0.0f;
  if (rh > 100.0f) return 100.0f;
  return rh;
}

float vpdAir(float tempC, float rh) {
  rh = clampRh(rh);
  return vpdSvp(tempC) * (1.0f - rh / 100.0f);
}

float vpdLeaf(float tempC, float rh, float leafOffsetC) {
  rh = clampRh(rh);
  return vpdSvp(tempC - leafOffsetC) - vpdSvp(tempC) * rh / 100.0f;
}

bool vpdInputValid(float tempC, float rh) {
  return isfinite(tempC) && isfinite(rh);
}
