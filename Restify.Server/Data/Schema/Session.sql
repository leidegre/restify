
create table [Session] (
    [SessionId] binary(32) not null,
    [UserId] binary(32) not null,
    [AirTime] bigint not null,
    primary key ([SessionId], [UserId])
)
;
