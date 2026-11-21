CREATE OR REPLACE FUNCTION pg_wget(text)
RETURNS text
AS '$libdir/pg_wget', 'pg_wget'
LANGUAGE C STRICT VOLATILE;
