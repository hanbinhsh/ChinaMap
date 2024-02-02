create database ChinaMap;
SET SQL_SAFE_UPDATES = 0;
use ChinaMap;

create table user(
userid varchar(25) primary key,
userpassword varchar(50),
favoritePlace varchar(50)
);

create table log(
logid int primary key,
userid varchar(25),
startplace varchar(50),
endplace varchar(50),
usedalgorithm varchar(50),
foreign key (userid) references user(userid)
);

create table suggest(
place varchar(50) primary key,
message varchar(1000)
);

#更改用户名
#插入新用户名的用户。将老用户名的日志名称修改后插入日志，删除老用户名的日志，删除老用户即可
drop procedure if exists UpdateUserName;
delimiter $$
create procedure UpdateUserName(oldid varchar(25), newid varchar(25))
begin
declare logidnumber int;
declare flag boolean default true;
declare c cursor for select logid from log where userid = oldid;
declare continue handler for not found set flag = false;
open c;
fetch c into logidnumber;
insert into user values(newid,
	(select T.userpassword from(select userpassword from user where userid = oldid) T),
    (select T.favoritePlace from(select favoritePlace from user where userid = oldid) T));
while flag=true do
	insert into log values(
		((select T.logid from(select max(logid) logid from log) T)+1),
		newid,
		(select T.startplace from(select startplace from log where logid = logidnumber) T),
		(select T.endplace from(select endplace from log where logid = logidnumber) T),
		(select T.usedalgorithm from(select usedalgorithm from log where logid = logidnumber) T));
	fetch c into logidnumber;
end while;
close c;
call DeleteUser(oldid);
end$$
delimiter ;

#插入日志记录时更新用户最爱地点
delimiter $$
create trigger t
after insert on log
for each row
begin
update user set favoritePlace = (
	select endplace from log
	where userid = new.userid
	group by userid,endplace
	order by count(endplace) desc
    limit 0,1
) where userid = new .userid;
end $$
delimiter ;

#删除用户日志
drop procedure if exists DeleteUserData;
delimiter $$
create procedure DeleteUserData(id varchar(25))
begin
declare uid varchar(25);
declare flag boolean default true;
declare c cursor for select userid from log where userid = id;
declare continue handler for not found set flag = false;
open c;
fetch c into uid;
while flag=true do
	delete log from log where userid = uid;
	fetch c into uid;
end while;
close c;
update user set favoritePlace = '' where userid = id;
end$$
delimiter ;

#删除用户
drop procedure if exists DeleteUser;
delimiter $$
create procedure DeleteUser(id varchar(25))
begin
call DeleteUserData(id);
delete user from user where userid = id;
end$$
delimiter ;

#重置推荐
drop procedure if exists ResetSuggest;
delimiter $$
create procedure ResetSuggest()
begin
truncate suggest;
INSERT INTO suggest (place, message) VALUES
('北京', '北京:故宫、天安门、长城等历史文化景点，美食文化丰富'),
('成都', '成都:宽窄巷子、武侯祠、大熊猫基地等独特景点'),
('福州', '福州:鼓山、三坊七巷、福建博物院等景点'),
('广州', '广州:珠江夜游、陈家祠、白云山等多元文化景点'),
('贵阳', '贵阳:黔灵公园、花溪公园、青岩古镇等景点'),
('哈尔滨', '哈尔滨:索菲亚大教堂、太阳岛、冰雪大世界等景点'),
('海口', '海口:分界洲岛、呀诺达雨林、骑楼老街等景点'),
('杭州', '杭州:西湖、灵隐寺、宋城等自然与人文景观'),
('合肥', '合肥:庐州府、包公祠、巢湖等历史文化景点'),
('呼和浩特', '呼和浩特:成吉思汗广场、呼和浩特大剧院、大召寺等景点'),
('济南', '济南:大明湖、千佛山、趵突泉等自然与人文景观'),
('昆明', '昆明:翠湖、西山、大观楼等自然风光和历史景点'),
('拉萨', '拉萨:布达拉宫、大昭寺、罗布林卡等藏传佛教圣地'),
('兰州', '兰州:黄河铁桥、兰州水车博物馆、中山桥等景点'),
('南昌', '南昌:滕王阁、青云谱、八一起义纪念馆等历史景点'),
('南京', '南京:中山陵、夫子庙、明故宫等历史名胜'),
('南宁', '南宁:青秀山、邕江、明秀路等自然与人文景观'),
('上海', '上海:外滩、东方明珠、豫园等现代与传统结合的地标'),
('沈阳', '沈阳:故宫、沈阳故宫、北陵公园等历史文化景点'),
('石家庄', '石家庄:赵州桥、正定古镇、井陉矿区等历史景点'),
('太原', '太原:晋祠、云冈石窟、平遥古城等历史遗迹'),
('天津', '天津:意式风情街、海河、古文化街区等景点'),
('乌鲁木齐', '乌鲁木齐:天山天池、南山牧场、大巴扎等景点'),
('武汉', '武汉:黄鹤楼、东湖、户部巷等历史风貌街区'),
('西安', '西安:兵马俑、大雁塔、古城墙等历史遗迹'),
('西宁', '西宁:青海湖、塔尔寺、茶卡盐湖等自然景观'),
('银川', '银川:沙湖、镇北堡、沙坡头等自然与历史景观'),
('长春', '长春:南湖公园、净月潭、伪满皇宫等文化景观'),
('长沙', '长沙:岳麓山、橘子洲头、湖南省博物馆等景点'),
('郑州', '郑州:黄河游览区、博物院、嵩山少林寺等景点'),
('重庆', '重庆:洪崖洞、长江索道、三峡博物馆等独特景点'),
('台北', '台北:故宫、象山、台北101等文化与现代景点'),
('香港', '香港:维多利亚港、迪士尼乐园、太平山等景点'),
('澳门', '澳门:威尼斯人、大三巴牌坊、澳门塔等景点');
end$$
delimiter ;

call ResetSuggest;

