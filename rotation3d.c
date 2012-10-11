#include "rotation3d.h"

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(rotation3d_in);

Datum
rotation3d_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    double r11, r12, r13, r21, r22, r23, r31, r32, r33;
    Rotation3D *result;

    if (sscanf(str, " ( %lf , %lf , %lf , %lf , %lf , %lf ,%lf , %lf , %lf )", &r11, &r12, &r13, &r21, &r22, &r23, &r31, &r32, &r33) != 9)
        ereport(ERROR,
            (errmsg("invalid input syntax for rotation3d: \"%s\"",
                str)));

    result = (Rotation3D*) palloc(sizeof(Rotation3D));
    result->r11 = r11;
    result->r12 = r12;
    result->r13 = r13;
    result->r21 = r21;
    result->r22 = r22;
    result->r23 = r23;
    result->r31 = r31;
    result->r32 = r32;
    result->r33 = r33;
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(rotation3d_out);

Datum
rotation3d_out(PG_FUNCTION_ARGS)
{
    Rotation3D * rotation3d = (Rotation3D*)PG_GETARG_POINTER(0);
    char * result;

    result = (char*)palloc(100);
    snprintf(result, 100, "Rotation3D (%.18g %.18g %.18g %.18g %.18g %.18g %.18g %.18g %.18g)", 
        rotation3d->r11, rotation3d->r12, rotation3d->r13,
        rotation3d->r21, rotation3d->r22, rotation3d->r23,
        rotation3d->r31, rotation3d->r22, rotation3d->r33);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(rotation3d_recv);

Datum
rotation3d_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
    Rotation3D * result;

    result = (Rotation3D*)palloc(sizeof(Rotation3D));
    result->r11 = pq_getmsgfloat8(buf);
    result->r12 = pq_getmsgfloat8(buf);
    result->r13 = pq_getmsgfloat8(buf);
    result->r21 = pq_getmsgfloat8(buf);
    result->r22 = pq_getmsgfloat8(buf);
    result->r23 = pq_getmsgfloat8(buf);
    result->r31 = pq_getmsgfloat8(buf);
    result->r32 = pq_getmsgfloat8(buf);
    result->r33 = pq_getmsgfloat8(buf);
   PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(rotation3d_send);

Datum
rotation3d_send(PG_FUNCTION_ARGS)
{
    Rotation3D * rotation3d = (Rotation3D*)PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendfloat8(&buf, rotation3d->r11);
    pq_sendfloat8(&buf, rotation3d->r12);
    pq_sendfloat8(&buf, rotation3d->r13);
    pq_sendfloat8(&buf, rotation3d->r21);
    pq_sendfloat8(&buf, rotation3d->r22);
    pq_sendfloat8(&buf, rotation3d->r23);
    pq_sendfloat8(&buf, rotation3d->r31);
    pq_sendfloat8(&buf, rotation3d->r32);
    pq_sendfloat8(&buf, rotation3d->r33);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(rotation3d_make);

Datum
rotation3d_make(PG_FUNCTION_ARGS)
{
    float8 r11 = PG_GETARG_FLOAT8(0);
    float8 r12 = PG_GETARG_FLOAT8(1);
    float8 r13 = PG_GETARG_FLOAT8(2);
    float8 r21 = PG_GETARG_FLOAT8(3);
    float8 r22 = PG_GETARG_FLOAT8(4);
    float8 r23 = PG_GETARG_FLOAT8(5);
    float8 r31 = PG_GETARG_FLOAT8(6);
    float8 r32 = PG_GETARG_FLOAT8(7);
    float8 r33 = PG_GETARG_FLOAT8(8);

    Rotation3D * result;

    result = (Rotation3D*) palloc(sizeof(Rotation3D));
    result->r11 = r11;
    result->r12 = r12;
    result->r13 = r13;
    result->r21 = r21;
    result->r22 = r22;
    result->r23 = r23;
    result->r31 = r31;
    result->r32 = r32;
    result->r33 = r33;
    PG_RETURN_POINTER(result);
}

