#include "postgres.h"
#include "liblwgeom.h"
#include "fmgr.h"

#include "point3d.h"
#include "rotation3d.h"

#include <math.h>

PG_MODULE_MAGIC;

const double dtr = M_PI/180.0;
const double wgs84asq = WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS;
const double wgs84bsq = WGS84_MINOR_AXIS*WGS84_MINOR_AXIS;
const double wgs84eccsq = (WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS - WGS84_MINOR_AXIS*WGS84_MINOR_AXIS)/(WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS);

#ifndef _GNU_SOURCE
inline void sincos(double theta, double *_sin, double *_cos);
inline void sincos(double theta, double *_sin, double *_cos)
{
    *_sin = sin(theta);
    *_cos = cos(theta);
}
#endif

void radcur (double lat, double rrnrm[3]);
double gc2gd (double flatgc, double alt);
double rearth(double lat);
Datum KM_ToECEF (PG_FUNCTION_ARGS);
Datum KM_ToLLA (PG_FUNCTION_ARGS);
Datum KM_ToENU (PG_FUNCTION_ARGS);
Datum KM_Rotate (PG_FUNCTION_ARGS);

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

PG_FUNCTION_INFO_V1(KM_ToECEF);

Datum KM_ToECEF (PG_FUNCTION_ARGS)
{
    GSERIALIZED *geom;
    LWPOINT *lwpoint;
    int32_t srid;
    POINT3DZ p;

    double flat, flon;
    double clat, clon, slat, slon;
    double rrnrm [3];
    double rn;

    Point3D * ecef;

    geom = (GSERIALIZED*)PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0));
    srid = gserialized_get_srid(geom);

    if (srid != 4326)
    {
        PG_FREE_IF_COPY(geom, 0);
        elog(ERROR, "Input geometry has SRID %d.  4326 required.", srid);
        PG_RETURN_NULL();
    }

    if (gserialized_get_type(geom) != POINTTYPE)
    {
        PG_FREE_IF_COPY(geom, 0);
        elog(ERROR, "Input geometry must be a point.");
        PG_RETURN_NULL();
    }

    if (!gserialized_has_z(geom))
    {
        PG_FREE_IF_COPY(geom, 0);
        elog(ERROR, "Input geometry must have z coordinate.");
        PG_RETURN_NULL();
    }

    lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom));

    lwpoint_getPoint3dz_p(lwpoint, &p);

    flat = dtr*p.y;
    flon = dtr*p.x;

    sincos(flat, &slat, &clat);
    sincos(flon, &slon, &clon);

    radcur(flat, rrnrm);
    rn = rrnrm[1];

    ecef = (Point3D *)palloc(sizeof(Point3D));

    if (!ecef)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    ecef->x = (rn + p.z) * clat * clon;
    ecef->y = (rn + p.z) * clat * slon;
    ecef->z = ((1.0 - wgs84eccsq)*rn + p.z) * slat;

    return PointerGetDatum(ecef);
}

PG_FUNCTION_INFO_V1(KM_ToLLA);

Datum KM_ToLLA (PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

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

    LWGEOM * lwpoint = NULL;
    GSERIALIZED * result = NULL;

    size_t ret_size;

    rp = sqrt(point3d->x*point3d->x + point3d->y*point3d->y + point3d->z*point3d->z);

    flatgc = asin(point3d->z/rp);            

    testval = fabs(point3d->x) + fabs(point3d->y);
    if (testval < 1.0e-10)
        flon = 0.0;
    else
        flon = atan2(point3d->y, point3d->x);

    if (flon < 0.0)
        flon += 2.0*M_PI;

    p = sqrt(point3d->x*point3d->x + point3d->y*point3d->y);

    if (p < 1.0e-10)
    {
        flat = M_PI/2.0;
        if (point3d->z < 0.0)
            flat = -M_PI/2.0;

        alt = rp - rearth(flat);

        if (flon > M_PI)
            flon -= 2.0*M_PI;
        
        lwpoint = (LWGEOM*)lwpoint_make3dz(4326, flon/dtr, flat/dtr, alt);
        
        if (lwgeom_is_empty(lwpoint))
            PG_RETURN_NULL();

    	ret_size = 0;
	    result = gserialized_from_lwgeom(lwpoint, 0, &ret_size);
    	if ( ! result ) 
            lwerror("Unable to serialize lwgeom.");
	    SET_VARSIZE(result, ret_size);

        lwgeom_free(lwpoint);

        PG_RETURN_POINTER(result);
    }

    rnow = rearth(flatgc);
    alt = rp - rnow;
    flat = gc2gd(flatgc, alt);

    radcur(flat, rrnrm);
    rn = rrnrm[1];

    for (kount = 0; kount < 5; ++kount)
    {
        slat = sin(flat);
        tangd = (point3d->z + rn*wgs84eccsq*slat)/p;
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

    lwpoint = (LWGEOM*)lwpoint_make3dz(4326, flon/dtr, flat/dtr, alt);
        
    if (lwgeom_is_empty(lwpoint))
        PG_RETURN_NULL();

    ret_size = 0;
    result = gserialized_from_lwgeom(lwpoint, 0, &ret_size);
    if ( ! result ) 
        lwerror("Unable to serialize lwgeom.");
    SET_VARSIZE(result, ret_size);

    lwgeom_free(lwpoint);

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(KM_ToENU);

Datum KM_ToENU (PG_FUNCTION_ARGS)
{
    GSERIALIZED *geom;
    LWPOINT *lwpoint;
    int32_t srid;
    POINT2D p;
    double flat, flon;
    double clat, slat, clon, slon;
    Rotation3D * rotation;

    geom = (GSERIALIZED*)PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0));
    srid = gserialized_get_srid(geom);

    if (srid != 4326)
    {
        PG_FREE_IF_COPY(geom, 0);
        elog(ERROR, "Input geometry has SRID %d.  4326 required.", srid);
        PG_RETURN_NULL();
    }

    if (gserialized_get_type(geom) != POINTTYPE)
    {
        PG_FREE_IF_COPY(geom, 0);
        elog(ERROR, "Input geometry must be a point.");
        PG_RETURN_NULL();
    }

    lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom));

    lwpoint_getPoint2d_p(lwpoint, &p);

    flat = dtr*p.y;
    flon = dtr*p.x;

    sincos(flat, &slat, &clat);
    sincos(flon, &slon, &clon);

    rotation = (Rotation3D*)palloc(sizeof(Rotation3D));

    if (!rotation)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    rotation->r11 = -slon;
    rotation->r12 = -clon*slat;
    rotation->r13 = clat*clon;
    rotation->r21 = clon;
    rotation->r22 = -slon*slat;
    rotation->r23 = slon*clat;
    rotation->r31 = 0;
    rotation->r32 = clat;
    rotation->r33 = slat;

    return PointerGetDatum(rotation);
}

PG_FUNCTION_INFO_V1(KM_Rotate);

Datum KM_Rotate (PG_FUNCTION_ARGS)
{
    Rotation3D * rotation3d = (Rotation3D*)PG_GETARG_POINTER(0);
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(1);
    bool inverse = PG_GETARG_BOOL(2);

    Point3D * result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    if (inverse)
    {
        result->x = rotation3d->r11*point3d->x + rotation3d->r21*point3d->y + rotation3d->r31*point3d->z;
        result->y = rotation3d->r12*point3d->x + rotation3d->r22*point3d->y + rotation3d->r32*point3d->z;
        result->z = rotation3d->r13*point3d->x + rotation3d->r23*point3d->y + rotation3d->r33*point3d->z;
    }
    else
    {
        result->x = rotation3d->r11*point3d->x + rotation3d->r12*point3d->y + rotation3d->r13*point3d->z;
        result->y = rotation3d->r21*point3d->x + rotation3d->r22*point3d->y + rotation3d->r23*point3d->z;
        result->z = rotation3d->r31*point3d->x + rotation3d->r32*point3d->y + rotation3d->r33*point3d->z;
    }

    return PointerGetDatum(result);
}
