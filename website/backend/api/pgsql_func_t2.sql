create or replace function pg_temp.t2(iuser int)
returns TABLE
(
	i_user int,
	s_account varchar
) as $t2$
declare
	v_i varchar;
begin
	v_i = iuser;
	RETURN QUERY 
	select a.i_user, a.s_account from t_bas_user a where a.i_user >= v_i::int;
end $t2$ LANGUAGE plpgsql IMMUTABLE;
