-- Exceptions
too_big : exception;
procedure check(n : integer) is
begin
   if n > 100 then raise too_big; end if;
   putint(n); put_line(" is fine");
end check;
check(42);
begin check(500); exception when too_big => put_line("too_big was raised and handled"); end;
v : array(1..3) of integer;
v(5) := 1;
where;
