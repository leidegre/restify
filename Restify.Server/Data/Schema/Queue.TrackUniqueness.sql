
--we allow each user to queue the the same track once
--as long as it's in the queue it cannot be queued again
CREATE UNIQUE NONCLUSTERED INDEX IX_Queue_Track ON [Queue] ([TrackId], [UserId])
;