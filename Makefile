EXTENSION = pg_wget
DATA = pg_wget--1.0.sql
MODULES = pg_wget

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
