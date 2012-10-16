DROP TYPE Point3D CASCADE;

CREATE TYPE Point3D;

CREATE OR REPLACE FUNCTION point3d_in(cstring)
    RETURNS Point3D
    AS 'point3d', 'point3d_in'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION point3d_out(Point3D)
    RETURNS cstring
    AS 'point3d', 'point3d_out'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION point3d_recv(internal)
    RETURNS Point3D
    AS 'point3d', 'point3d_recv'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION point3d_send(Point3D)
    RETURNS bytea
    AS 'point3d', 'point3d_send'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE Point3D (
    internallength = 24,
    input = point3d_in,
    output = point3d_out,
    receive = point3d_recv,
    send = point3d_send,
    alignment = double
);

CREATE OR REPLACE FUNCTION KM_Point3D(double precision, double precision, double precision)
    RETURNS Point3D
    AS 'point3d', 'point3d_make'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_X(Point3D)
    RETURNS double precision
    AS 'point3d', 'point3d_x'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Y(Point3D)
    RETURNS double precision
    AS 'point3d', 'point3d_y'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Z(Point3D)
    RETURNS double precision
    AS 'point3d', 'point3d_z'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Add(Point3D, Point3D)
    RETURNS Point3D
    AS 'point3d', 'point3d_add'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Subtract(Point3D, Point3D)
    RETURNS Point3D
    AS 'point3d', 'point3d_subtract'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Scale(Point3D, double precision)
    RETURNS Point3D
    AS 'point3d', 'point3d_scale'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Negate(Point3D)
    RETURNS Point3D
    AS 'point3d', 'point3d_negate'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Direction(Point3D, Point3D)
    RETURNS Point3D
    AS 'point3d', 'point3d_direction'
    LANGUAGE C IMMUTABLE STRICT;

------------------------------------------------------------------------------------------

DROP TYPE Rotation3D CASCADE;

CREATE TYPE Rotation3D;

CREATE OR REPLACE FUNCTION rotation3d_in(cstring)
    RETURNS Rotation3D
    AS 'rotation3d', 'rotation3d_in'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION rotation3d_out(Rotation3D)
    RETURNS cstring
    AS 'rotation3d', 'rotation3d_out'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION rotation3d_recv(internal)
    RETURNS Rotation3D
    AS 'rotation3d', 'rotation3d_recv'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION rotation3d_send(Rotation3D)
    RETURNS bytea
    AS 'rotation3d', 'rotation3d_send'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE Rotation3D (
    internallength = 72,
    input = rotation3d_in,
    output = rotation3d_out,
    receive = rotation3d_recv,
    send = rotation3d_send,
    alignment = double
);

CREATE OR REPLACE FUNCTION KM_Rotation3D(double precision, double precision, double precision,
                                         double precision, double precision, double precision,
                                         double precision, double precision, double precision)
    RETURNS Rotation3D
    AS 'rotation3d', 'rotation3d_make'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION KM_Explode(Rotation3d)
    RETURNS double precision []
    AS 'rotation3d', 'rotation3d_explode'
    LANGUAGE C IMMUTABLE STRICT;

------------------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION KM_ToENU(geometry(Point,4326))
    RETURNS Rotation3D
    AS 'pg_lla_ecef', 'KM_ToENU'
    LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION KM_ToECEF(geometry(Point,4326), double precision)
    RETURNS Point3D
    AS 'pg_lla_ecef', 'KM_ToECEF'
    LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION KM_ToLLA(Point3d)
    RETURNS geometry(Point,4326)
    AS 'pg_lla_ecef', 'KM_ToLLA'
    LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION KM_Rotate(Rotation3D, Point3D, boolean)
    RETURNS Point3D
    AS 'pg_lla_ecef', 'KM_Rotate'
    LANGUAGE C STRICT;
