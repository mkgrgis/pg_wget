Simple `wget` data downloader for PostgreSQL
============================================

This is a [PostgreSQL](https://www.postgresql.org/) extension which allows to read `wget` program output value in PostgreSQL. This extension works with all of PostgreSQL 8+.

<img src="https://upload.wikimedia.org/wikipedia/commons/2/29/Postgresql_elephant.svg" align="center" height="100" alt="PostgreSQL"/>	+	WGET program

Current CI status.

[![CI Status](https://github.com/mkgrgis/pg_wget/actions/workflows/CI.yml/badge.svg)](https://github.com/mkgrgis/pg_wget/actions/workflows/CI.yml)

Contents
--------

1. [Features](#features)
2. [Supported platforms](#supported-platforms)
3. [Installation](#installation)
4. [Functions](#functions)
5. [Examples](#examples)
6. [Limitations](#limitations)
7. [Tests](#tests)
8. [Contributing](#contributing)
9. [Useful links](#useful-links)

Features
--------

### Common features

This extension reads `wget` output from given URL as UTF-8 value in a PostgreSQL.
This is simplified analog of popular PostgreSQL extension `http` which doesn't
allow download, for example, Wikipedia markup data.

Example usage with Wikipedia raw markup URL:
```sql
SELECT pg_wget('https://ru.wikipedia.org/w/index.php?title=Бирюлёвский_дендрарий&action=raw');
```

Supported platforms
-------------------

`pg_wget` was developed on Linux and should run on any
reasonably POSIX-compliant system.

Installation
------------

### Package installation

There are no Linux distributives contains internal package with `pg_wget`.

Known special runtime dependencies: `wget` program.

### Source installation

Prerequisites:
* `gcc`
* `make`
* `postgresql-server-dev`, especially `postgres.h`

#### 1. Install Postgres development libraries

For Debian or Ubuntu:

`apt-get install libzip-dev libxml2-dev -y`

`apt-get install postgresql-server-dev-XX -y`, where XX matches your postgres version, i.e. `apt-get install postgresql-server-dev-19 -y`

#### 2. Build and install pg_wget

Before building please add a directory of `pg_config` to PATH or ensure `pg_config` program is accessible from command line only by the name.

```sh
make USE_PGXS=1
make install USE_PGXS=1
```

If you want to build `pg_wget` in a source tree of PostgreSQL, use
```sh
make
make install
```

Functions
---------

- text **pg_wget**(url text);

Returns UTF-8 text value downloaded by the URL.

Examples
--------

### Install the extension:

Once for a database you need, as PostgreSQL superuser.

```sql
	CREATE EXTENSION pg_wget;
```

This is useful to use limited internet access for all DB users except to special:

``` sql
REVOKE EXECUTE ON FUNCTION pg_wget(text) FROM PUBLIC;
GANT EXECUTE ON FUNCTION pg_wget(text) TO ******; -- Use your own special user for downloading by URL
```

### Use Wikipedia article table as PostgreSQL relation:

```sql
-- Представление, осуществляющее скачку вики-разметки из Википедии
create view wget_wikipedia as 
select pg_wget('https://ru.wikipedia.org/w/index.php?title=Бирюлёвский_дендрарий&action=raw') t, now() dt;

-- Формирователь реляционного табличного вида описи дендрологической коллекции
with floral_t as ( -- Ракрывающаяся таблица в разделе «"Флора"»
select (regexp_match(t, '(?<=Опись дендрологической коллекции)[^}]+}'))[1] t
from wget_wikipedia
),
tab_okn as ( -- Вложенная таблица из паспорта ОКН
select (regexp_match(t,  '(?<={\|)[\s\S]+(?=\|})'))[1] t
from floral_t
),
wiki_tab as ( -- Пропускаем заголовок, читаем после «|-» с новой строки
select (regexp_matches(t,  '(?<=\|\-
\s?\|).+'))[1] t
from tab_okn
),
cortage as ( -- Переводим строки вики-разметки в набор строк
select regexp_split_to_table(t, '
\|\-
\|') c
from wiki_tab
),
arr as ( -- В каждой строке массив ячеек
select regexp_split_to_array(c, '
\|') a
from cortage
)
select split_part(a[1], '&', 1)::text "Адрес",
       split_part(a[1], '×', 1)::int2 "Уч.",
       split_part(split_part(a[1], '×', 2), '(', 1)::int2 "№",
       split_part(split_part(a[1], '×', 2), '&', 1) "№_",
       length(a[2]) > 1 "Утрата",
       a[3]::int2 "Видов",
       split_part(regexp_replace(a[4],'\[|\]', '', 'g'), '|', 1) "Вид или род wiki",
       split_part(regexp_replace(a[4],'\[|\]', '', 'g'), '|', -1) "Вид или род",
       split_part(regexp_replace(a[5],'\[|\]', '', 'g'), ' ', 1) "OSM URL",
       a[6] "обсадка",
       a[7] "сохранность",
       a[2] a2,
       a[4] a4,
       a[5] a5
from arr;
```

Limitations
-----------

No special limitations are known.


Contributing
------------

Opening issues and pull requests on GitHub are welcome.
For pull request, please make sure these items below for testing:
- Create test cases (if needed) for the latest version of PostgreSQL supported by `pg_wget`. All error testcases should have a comment about test purpose.
- Execute test cases and update expectations for the latest version of PostgreSQL
- Test creation and execution for other PostgreSQL versions are welcome but not required.

Preferred code style see in PostgreSQL source codes. For example

```C
type
funct_name (type arg ...)
{
	t1 var1 = value1;
	t2 var2 = value2;

	for (;;)
	{
	}
	if ()
	{
	}
}
```
Useful links
------------

### Source

 - https://pgxn.org/dist/pg_wget/

License
-------
* Copyright © 2026, mkgrgis

PostgreSQL license
Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.

See the [`License`](License) file for full details.
