use ChinaMap;

select endplace from log
where userid = "user001"
group by userid,endplace
order by count(endplace) desc;

select * from user;
select * from log;
select * from suggest;

truncate log;

select userid from user where userid = 'user001';

call DeleteUserData('a');

call UpdateUserName('c','bBasdc');