#ifndef LLA_ECEF_H
#define LLA_ECEF_H

void LLAToECEF (double lla[3], double ecef[3]);
void ECEFToLLA (double ecef[3], double lla[3]);
void LLAToENU (double lla[3], double enu[9]);

#endif
