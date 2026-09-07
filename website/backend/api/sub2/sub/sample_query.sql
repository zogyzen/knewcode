begin
	drop table if exists tab;
	CREATE TEMP TABLE tab as
	select * from test;

	:logMsg = 'log';
	:errMsg = 'errMsg';
end;
