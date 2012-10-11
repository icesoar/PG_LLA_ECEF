#include "point3d.h"

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(point3d_in);

Datum
point3d_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    double x, y, z;
    Point3D *result;

    if (sscanf(str, " ( %lf , %lf , %lf )", &x, &y, &z) != 3)
        ereport(ERROR,
            (errmsg("invalid input syntax for point3d: \"%s\"",
                str)));

    result = (Point3D*) palloc(sizeof(Point3D));
    result->x = x;
    result->y = y;
    result->z = z;
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_out);

Datum
point3d_out(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);
    char * result;

    result = (char*)palloc(100);
    snprintf(result, 100, "Point3D (%.18g %.18g %.18g)", point3d->x, point3d->y, point3d->z);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(point3d_recv);

Datum
point3d_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
    Point3D * result;

    result = (Point3D*)palloc(sizeof(Point3D));
    result->x = pq_getmsgfloat8(buf);
    result->y = pq_getmsgfloat8(buf);
    result->z = pq_getmsgfloat8(buf);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_send);

Datum
point3d_send(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendfloat8(&buf, point3d->x);
    pq_sendfloat8(&buf, point3d->y);
    pq_sendfloat8(&buf, point3d->z);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
