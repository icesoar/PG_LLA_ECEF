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

CREATE OR REPLACE FUNCTION point3d_send(Point3d)
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

CREATE OR REPLACE FUNCTION KM_ToECEF(geometry(Point,4326), double precision)
    RETURNS Point3D
    AS 'pg_lla_ecef', 'KM_ToECEF'
    LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION KM_ToLLA(Point3d)
    RETURNS geometry(Point,4326)
    AS 'pg_lla_ecef', 'KM_ToLLA'
    LANGUAGE C STRICT;

