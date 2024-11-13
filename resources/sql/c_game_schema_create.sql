/***************************************************************************************************
Procedure:          noats_db_ddl
Create Date:        2024-11-12
Author:             Oskar
Description:        data definition for noats project.
Call by:            [main.c, ...]
Affected table(s):  [public.noats]
Used By:            noats.c
Parameter(s):       [...]
Usage:              [...]
****************************************************************************************************
SUMMARY OF CHANGES
Date(yyyy-mm-dd)    Author              Comments
------------------- ------------------- ------------------------------------------------------------
2024-11-12          Oskar               prettied.
***************************************************************************************************/

CREATE TABLE IF NOT EXISTS public.noats (
    /* pkey */
    id                 bigint GENERATED ALWAYS AS IDENTITY,
    /* fkey */
    /* own fields */
    title              varchar(256),
    author             varchar(256),
    noat               varchar(256),
    fmt                varchar(256),
    body               text,
    created            timestamp default current_timestamp, /* [without timezone] */
    updated            timestamp, /* [without timezone] */
    /* constraints */
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS public.project (
    /* pkey */
    id                 bigint GENERATED ALWAYS AS IDENTITY,
    /* fkey */
    /* own fields */
    title              varchar(256),
    author             varchar(256),
    description        text,
    priority           smallint,
    created            timestamp default current_timestamp, /* [without timezone] */
    updated            timestamp, /* [without timezone] */
    /* constraints */
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS public.noats_project_assoc (
    /* pkey */
    id                 bigint GENERATED ALWAYS AS IDENTITY,
    /* fkey */
    noat_id            bigint,
    project_id         bigint,
    /* own fields */
    /* constraints */
    PRIMARY KEY (id),
    FOREIGN KEY (noat_id)    REFERENCES public.noats (id),
    FOREIGN KEY (project_id) REFERENCES public.project (id)
);

/**************************************************************************************************
 Functions
 --------------------------------------------------------------------------------------------------
 public.update_timestamp      <description>
 public.created_timestamp     <description>
 **************************************************************************************************/

CREATE OR REPLACE FUNCTION public.update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
   IF row(NEW.*) IS DISTINCT FROM row(OLD.*) THEN
      NEW.updated = now();
      RETURN NEW;
   ELSE
      RETURN OLD;
   END IF;
END;
$$ language 'plpgsql';

CREATE OR REPLACE FUNCTION public.created_timestamp()
RETURNS TRIGGER AS $$
BEGIN
  NEW.created := now();
  NEW.updated := now();
  RETURN NEW;
END;
$$ LANGUAGE 'plpgsql';

/**************************************************************************************************
 Triggers
 --------------------------------------------------------------------------------------------------
 **************************************************************************************************/

CREATE OR REPLACE TRIGGER
    create_noats_timestamp
BEFORE INSERT ON
    public.noats
FOR EACH ROW EXECUTE PROCEDURE
    public.created_timestamp();

CREATE OR REPLACE TRIGGER
    create_project_timestamp
BEFORE INSERT ON
    public.project
FOR EACH ROW EXECUTE PROCEDURE
    public.created_timestamp();

CREATE OR REPLACE TRIGGER
    update_noats_timestamp
BEFORE UPDATE ON
    public.noats
FOR EACH ROW EXECUTE PROCEDURE
    public.update_timestamp();

CREATE OR REPLACE TRIGGER
    update_project_timestamp
BEFORE UPDATE ON
    public.project
FOR EACH ROW EXECUTE PROCEDURE
    public.update_timestamp();
