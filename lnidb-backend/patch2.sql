-- patch for period cache [apr 2015]

-- new: period cache
drop table patperiod cascade;
create table patperiod (
       pid   integer references patients(id),
       first timestamp,
       last  timestamp,
       primary key (pid)
);

