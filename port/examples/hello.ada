put_line("Hello from 1984");
function fact(n : integer) return integer is
begin
   if n <= 1 then return 1; else return n * fact(n - 1); end if;
end fact;
putint(fact(10)); new_line;
fact(12)
pp(fact'sym);
