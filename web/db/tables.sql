create table members(
	userid int unsigned auto_increment primary key,
	username varchar(64) not null,
	firstname varchar(64),
	lastname varchar(64),
	role tinyint not null,
	tokenkey varchar(16),
	joineddate date
);

insert into members(username, firstname, role, joineddate) values ('admin', 'Administrator', 0, '2015-04-21');

create table search_history(
	keyword varchar(128) not null,
	searchts timestamp,
	userid int unsigned,
	constraint pk_SearchHistory primary key (keyword, searchts)
);