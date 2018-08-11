
CREATE FUNCTION SetSQLStreamingPlugin RETURNS INTEGER SONAME 'libsmysql.so'

select SetSQLStreamingPlugin('uid=root;pwd=Smash123')