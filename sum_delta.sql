use poker

select sum(delta)
from poker_sessions
where delta < 0;

select sum(delta)
from poker_sessions
where delta > 0;

select sit_and_go,sum(delta)
from poker_sessions
group by sit_and_go
order by sit_and_go;

select sum(delta)
from poker_sessions;

quit
