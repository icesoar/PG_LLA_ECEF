MODULES = pg_lla_ecef point3d

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
include $(PGXS)

all: pg_lla_ecef.so point3d.so

pg_lla_ecef.so: pg_lla_ecef.o
	gcc -shared -o pg_lla_ecef.so pg_lla_ecef.o /hydra/S1/kmatzen/homebrew/Cellar/postgresql/9.1.4/lib/postgis-2.0.so

pg_lla_ecef.o:  pg_lla_ecef.c
	gcc -o pg_lla_ecef.o -c pg_lla_ecef.c $(CFLAGS) -I$(INCLUDEDIR) -I/hydra/S1/kmatzen/homebrew/include

point3d.o:  point3d.c
	gcc -o point3d.o -c point3d.c $(CFLAGS) -I$(INCLUDEDIR) -I/hydra/S1/kmatzen/homebrew/include

point3d.so: point3d.o
	gcc -shared -o point3d.so point3d.o 