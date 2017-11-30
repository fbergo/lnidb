
-- patch for compression-related fields [dec 2014]

begin;
alter table files add column compressed boolean default false;
alter table files add column csize bigint default 0;
commit;
