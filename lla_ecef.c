#include "lla_ecef.h"

#include <math.h>

#define WGS84_MAJOR_AXIS 6378137.0
#define WGS84_INVERSE_FLATTENING 298.257223563
#define WGS84_MINOR_AXIS (WGS84_MAJOR_AXIS - WGS84_MAJOR_AXIS / WGS84_INVERSE_FLATTENING)
#define WGS84_RADIUS ((2.0 * WGS84_MAJOR_AXIS + WGS84_MINOR_AXIS ) / 3.0)

const double dtr = M_PI/180.0;
const double wgs84asq = WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS;
const double wgs84bsq = WGS84_MINOR_AXIS*WGS84_MINOR_AXIS;
const double wgs84eccsq = (WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS - WGS84_MINOR_AXIS*WGS84_MINOR_AXIS)/(WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS);

#ifndef _GNU_SOURCE
static inline void sincos(double theta, double *_sin, double *_cos);
inline void sincos(double theta, double *_sin, double *_cos)
{
    *_sin = sin(theta);
    *_cos = cos(theta);
}
#endif

static void radcur (double lat, double rrnrm[3]);
static double gc2gd (double flatgc, double alt);
static double rearth(double lat);

void radcur (double lat, double rrnrm[3])
{
    double clat, slat;
    double dsq, d, rn, rm, rho, rsq, r, z;

    sincos(lat, &slat, &clat);

    dsq = 1.0 - wgs84eccsq * slat * slat;
    d = sqrt(dsq);

    rn = WGS84_MAJOR_AXIS/d;
    rm = rn * (1.0 - wgs84eccsq)/dsq;

    rho = rn * clat;
    z = (1.0 - wgs84eccsq) * rn * slat;
    rsq = rho*rho + z*z;
    r = sqrt(rsq);

    rrnrm[0] = r;
    rrnrm[1] = rn;
    rrnrm[2] = rm;
}

double gc2gd (double flatgc, double alt)
{
    double flatgd;
    double rrnrm [3];
    double rn;
    double tlat;
    double altnow, ratio;

    altnow = alt;

    radcur(flatgc, rrnrm);
    rn = rrnrm[1];

    ratio = 1.0 - wgs84eccsq*rn/(rn+altnow);

    tlat = tan(flatgc)/ratio;
    flatgd = atan(tlat);

    radcur(flatgd, rrnrm);
    rn = rrnrm[1];

    ratio = 1.0 - wgs84eccsq*rn/(rn+altnow);
    tlat = tan(flatgc)/ratio;
    flatgd = atan(tlat);

    return flatgd;
}

double rearth(double lat)
{
    double rrnrm[3];
    double r;

    radcur(lat, rrnrm);
    r = rrnrm[0];

    return r;
}

void LLAToECEF (double lla[3], double ecef[3])
{
    double flat, flon;
    double clat, clon, slat, slon;
    double rrnrm [3];
    double rn;

    flat = dtr*lla[0];
    flon = dtr*lla[1];

    sincos(flat, &slat, &clat);
    sincos(flon, &slon, &clon);

    radcur(flat, rrnrm);
    rn = rrnrm[1];

    ecef[0] = (rn + lla[2]) * clat * clon;
    ecef[1] = (rn + lla[2]) * clat * slon;
    ecef[2] = ((1.0 - wgs84eccsq)*rn + lla[2]) * slat;
}

void ECEFToLLA (double ecef[3], double lla[3])
{
    double flatgc, flatn, dlat;
    double rnow, rp;
    double p;
    int kount;
    double tangd;

    double testval;
    
    double rn;
    double clat, slat;
    double rrnrm [3];

    double flat, flon, alt;

    rp = sqrt(ecef[0]*ecef[0] + ecef[1]*ecef[1] + ecef[2]*ecef[2]);

    flatgc = asin(ecef[2]/rp);

    testval = fabs(ecef[0]) + fabs(ecef[1]);
    if (testval < 1.0e-10)
        flon = 0.0;
    else
        flon = atan2(ecef[1], ecef[0]);

    if (flon < 0.0)
        flon += 2.0*M_PI;

    p = sqrt(ecef[0]*ecef[0] + ecef[1]*ecef[1]);

    if (p < 1.0e-10)
    {
        flat = M_PI/2.0;
        if (ecef[2] < 0.0)
            flat = -M_PI/2.0;

        alt = rp - rearth(flat);

        if (flon > M_PI)
            flon -= 2.0*M_PI;
       
        lla[0] = flat/dtr;
        lla[1] = flon/dtr;
        lla[2] = alt; 
        return;
    }

    rnow = rearth(flatgc);
    alt = rp - rnow;
    flat = gc2gd(flatgc, alt);

    radcur(flat, rrnrm);
    rn = rrnrm[1];

    for (kount = 0; kount < 5; ++kount)
    {
        slat = sin(flat);
        tangd = (ecef[2] + rn*wgs84eccsq*slat)/p;
        flatn = atan(tangd);
    
        dlat = flatn - flat;
        flat = flatn;
        clat = cos(flat);

        radcur(flat, rrnrm);
        rn = rrnrm[1];

        alt = (p/clat) - rn;

        if (fabs(dlat) < 1.0e-12)
            break;
    }

    if (flon > M_PI)
        flon -= 2.0*M_PI;

    lla[0] = flat/dtr;
    lla[1] = flon/dtr;
    lla[2] = alt;
}

void LLAToENU (double lla[3], double enu[9])
{
    double flat, flon;
    double clat, slat, clon, slon;

    flat = dtr*lla[0];
    flon = dtr*lla[1];

    sincos(flat, &slat, &clat);
    sincos(flon, &slon, &clon);

    enu[0] = -slon;
    enu[1] = -clon*slat;
    enu[2] = clat*clon;
    enu[3] = clon;
    enu[4] = -slon*slat;
    enu[5] = slon*clat;
    enu[6] = 0;
    enu[7] = clat;
    enu[8] = slat;
}
