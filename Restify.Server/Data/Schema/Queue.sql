
create table [Queue] (
    [Timestamp] bigint not null,
    [TrackId] nchar(24) not null,
    [UserId] binary(32) not null,
    primary key ([Timestamp], [TrackId])
)
;
