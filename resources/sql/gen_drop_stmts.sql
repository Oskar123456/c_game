select
    string_agg('drop table ' || table_name || ' cascade;', ' ') as drop_all_sql
from
    information_schema.tables
where
    table_schema ilike 'public';


