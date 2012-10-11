#include "postgres.h"
#include "liblwgeom.h"
#include "fmgr.h"

#include "point3d.h"
#include "rotation3d.h"

#include <math.h>

PG_MODULE_MAGIC;

#define FP_NAN sqrt(-1)

const double dtr = M_PI/180.0;
const double wgs84eccsq = 1.0 - WGS84_MINOR_AXIS*WGS84_MINOR_AXIS/(WGS84_MAJOR_AXIS*WGS84_MAJOR_AXIS);

void radcur (double lat, double rrnrm[3])
{
    double a = FP_NAN, b = FP_NAN;
    double asq = FP_NAN, bsq = FP_NAN, ecc = FP_NAN, clat = FP_NAN, slat = FP_NAN;
    double dsq = FP_NAN, d = FP_NAN, rn = FP_NAN, rm = FP_NAN, rho = FP_NAN, rsq = FP_NAN, r = FP_NAN, z = FP_NAN;

    a = WGS84_MAJOR_AXIS;
    b = WGS84_MINOR_AXIS;

    asq = a*a;
    bsq = b*b;
    ecc = sqrt(wgs84eccsq);

    clat = cos(lat);
    slat = sin(lat);

    dsq = 1.0 - wgs84eccsq * slat * slat;
    d = sqrt(dsq);

    rn = a/d;
    rm = rn * (1.0 - wgs84eccsq)/dsq;

    rho = rn * clat;
    z = (1.0 - wgs84eccsq) * rn * slat;
    rsq = rho*rho + z*z;
    r = sqrt(rsq);

    rrnrm[0] = r;
    rrnrm[1] = rn;
    rrnrm[2] = rm;
}

double gc2gd (flatgc, alt)
{
    double flatgd = FP_NAN;
    double rrnrm [3] = {FP_NAN};
    double re = FP_NAN, rn = FP_NAN;
    double slat = FP_NAN, clat = FP_NAN, tlat = FP_NAN;
    double altnow = FP_NAN, ratio = FP_NAN;

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

double rearth(lat)
{
    double rrnrm[3] = {FP_NAN};
    double r = FP_NAN;

    radcur(lat, rrnrm);
    r = rrnrm[0];

    return r;
}

PG_FUNCTION_INFO_V1(KM_ToECEF);

Datum
KM_ToECEF (PG_FUNCTION_ARGS)
{
    GSERIALIZED *geom;
    LWPOINT *lwpoint;
    int32_t srid;
    POINT2D p;
    float8 alt = PG_GETARG_FLOAT8(1);

    double flat, flon;
    double clat, clon, slat, slon;
    double rrnrm [3];
    double rn, esq;
    double re;

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

    clat = cos(flat);
    slat = sin(flat);
    clon = cos(flon);
    slon = sin(flon);

    radcur(flat, rrnrm);
    rn = rrnrm[1];
    re = rrnrm[0];

    esq = wgs84eccsq;

    Point3D * ecef = (Point3D *)palloc(sizeof(Point3D));

    if (!ecef)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        return NULL;
    }

    ecef->x = (rn + alt) * clat * clon;
    ecef->y = (rn + alt) * clat * slon;
    ecef->z = ((1 - esq)*rn + alt) * slat;

    return PointerGetDatum(ecef);
}

PG_FUNCTION_INFO_V1(KM_ToLLA);

Datum
KM_ToLLA (PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    double flatgc = FP_NAN, flatn = FP_NAN, dlat = FP_NAN;
    double rnow = FP_NAN, rp = FP_NAN;
    double p = FP_NAN;
    int kount;
    double tangc = FP_NAN, tangd = FP_NAN;

    double testval = FP_NAN;
    
    double rn = FP_NAN;
    double clat = FP_NAN, slat = FP_NAN;
    double rrnrm [3] = {FP_NAN};

    double flat = FP_NAN, flon = FP_NAN, alt = FP_NAN;

    LWGEOM * lwpoint = NULL;
    GSERIALIZED * result = NULL;

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

        result = geometry_serialize(lwpoint);
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

    result = geometry_serialize(lwpoint);
    lwgeom_free(lwpoint);

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(KM_ToENU);

Datum
KM_ToENU (PG_FUNCTION_ARGS)
{
    GSERIALIZED *geom;
    LWPOINT *lwpoint;
    int32_t srid;
    POINT2D p;
    double flat, flon;
    double clat, slat, clon, slon;

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

    clat = cos(flat);
    slat = sin(flat);
    clon = cos(flon);
    slon = sin(flon);

    Rotation3D * rotation = (Rotation3D*)palloc(sizeof(Rotation3D));

    if (!rotation)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        return NULL;
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
