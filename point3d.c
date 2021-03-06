#include "point3d.h"

#include "postgres.h"
#include "catalog/pg_type.h"
#include "utils/array.h"
#include "fmgr.h"
#include "libpq/pqformat.h"

#include <math.h>

PG_MODULE_MAGIC;

Datum point3d_in(PG_FUNCTION_ARGS);
Datum point3d_out(PG_FUNCTION_ARGS);
Datum point3d_recv(PG_FUNCTION_ARGS);
Datum point3d_send(PG_FUNCTION_ARGS);
Datum point3d_make(PG_FUNCTION_ARGS);
Datum point3d_x(PG_FUNCTION_ARGS);
Datum point3d_y(PG_FUNCTION_ARGS);
Datum point3d_z(PG_FUNCTION_ARGS);
Datum point3d_add(PG_FUNCTION_ARGS);
Datum point3d_subtract(PG_FUNCTION_ARGS);
Datum point3d_dot(PG_FUNCTION_ARGS);
Datum point3d_direction(PG_FUNCTION_ARGS);
Datum point3d_negate(PG_FUNCTION_ARGS);
Datum point3d_scale(PG_FUNCTION_ARGS);
Datum point3d_explode(PG_FUNCTION_ARGS);
Datum point3d_norm(PG_FUNCTION_ARGS);
Datum point3d_normalize(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(point3d_in);

Datum point3d_in(PG_FUNCTION_ARGS)
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

Datum point3d_out(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);
    char * result;

    result = (char*)palloc(100);
    snprintf(result, 100, "Point3D (%.18g %.18g %.18g)", point3d->x, point3d->y, point3d->z);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(point3d_recv);

Datum point3d_recv(PG_FUNCTION_ARGS)
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

Datum point3d_send(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendfloat8(&buf, point3d->x);
    pq_sendfloat8(&buf, point3d->y);
    pq_sendfloat8(&buf, point3d->z);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(point3d_make);

Datum point3d_make(PG_FUNCTION_ARGS)
{
    float8 x = PG_GETARG_FLOAT8(0);
    float8 y = PG_GETARG_FLOAT8(1);
    float8 z = PG_GETARG_FLOAT8(2);

    Point3D * result;

    result = (Point3D*) palloc(sizeof(Point3D));
    result->x = x;
    result->y = y;
    result->z = z;
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_x);

Datum point3d_x(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    PG_RETURN_FLOAT8(point3d->x);
}

PG_FUNCTION_INFO_V1(point3d_y);

Datum point3d_y(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    PG_RETURN_FLOAT8(point3d->y);
}

PG_FUNCTION_INFO_V1(point3d_z);

Datum point3d_z(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    PG_RETURN_FLOAT8(point3d->z);
}

PG_FUNCTION_INFO_V1(point3d_add);

Datum point3d_add(PG_FUNCTION_ARGS)
{
    Point3D * a = (Point3D*)PG_GETARG_POINTER(0);
    Point3D * b = (Point3D*)PG_GETARG_POINTER(1);

    Point3D * result;
    
    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_subtract);

Datum point3d_subtract(PG_FUNCTION_ARGS)
{
    Point3D * a = (Point3D*)PG_GETARG_POINTER(0);
    Point3D * b = (Point3D*)PG_GETARG_POINTER(1);

    Point3D * result;
    
    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_dot);

Datum point3d_dot(PG_FUNCTION_ARGS)
{
    Point3D * a = (Point3D*)PG_GETARG_POINTER(0);
    Point3D * b = (Point3D*)PG_GETARG_POINTER(1);

    float8 result = a->x*b->x + a->y*b->y + a->z*b->z;

    PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(point3d_direction);

Datum point3d_direction(PG_FUNCTION_ARGS)
{
    Point3D * a = (Point3D*)PG_GETARG_POINTER(0);
    Point3D * b = (Point3D*)PG_GETARG_POINTER(1);

    Point3D * result;

    float8 norm;
    
    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = b->x - a->x;
    result->y = b->y - a->y;
    result->z = b->z - a->z;

    norm = sqrt(result->x*result->x + result->y*result->y + result->z*result->z);

    result->x /= norm;
    result->y /= norm;
    result->z /= norm;

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_negate);

Datum point3d_negate(PG_FUNCTION_ARGS)
{
    Point3D * in = (Point3D*)PG_GETARG_POINTER(0);

    Point3D * result;

    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = -in->x;
    result->y = -in->y;
    result->z = -in->z;

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_scale);

Datum point3d_scale(PG_FUNCTION_ARGS)
{
    Point3D * in = (Point3D*)PG_GETARG_POINTER(0);
    float8 scale = PG_GETARG_FLOAT8(1);

    Point3D * result;

    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = scale*in->x;
    result->y = scale*in->y;
    result->z = scale*in->z;

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(point3d_explode);

Datum point3d_explode(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    ArrayType * result;
    Datum * result_data;

    result_data = (Datum*)palloc(sizeof(float8)*3);

    if (!result_data)
    {
        PG_RETURN_NULL();
    }

    result_data[0] = Float8GetDatum(point3d->x);
    result_data[1] = Float8GetDatum(point3d->y);
    result_data[2] = Float8GetDatum(point3d->z);
    
    result = construct_array(result_data, 3, FLOAT8OID, sizeof(float8), true, 'd');

    pfree(result_data);

    PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(point3d_norm);

Datum point3d_norm(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    float8 norm = sqrt(point3d->x*point3d->x + point3d->y*point3d->y + point3d->z*point3d->z);

    PG_RETURN_FLOAT8(norm);
}

PG_FUNCTION_INFO_V1(point3d_normalize);

Datum point3d_normalize(PG_FUNCTION_ARGS)
{
    Point3D * point3d = (Point3D*)PG_GETARG_POINTER(0);

    float8 norm = sqrt(point3d->x*point3d->x + point3d->y*point3d->y + point3d->z*point3d->z);

    Point3D * result;

    result = (Point3D*)palloc(sizeof(Point3D));

    if (!result)
    {
        ereport(ERROR, (errmsg_internal("Out of virtual memory")));
        PG_RETURN_NULL();
    }

    result->x = point3d->x/norm;
    result->y = point3d->y/norm;
    result->z = point3d->z/norm;

    PG_RETURN_POINTER(result);
}
