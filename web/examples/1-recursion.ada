-- Recursion
function fib(n : integer) return integer is
begin
   if n < 2 then return n; end if;
   return fib(n - 1) + fib(n - 2);
end fib;
for i in 1..12 loop putint(fib(i)); putstr(" "); end loop;
new_line;
function gcd(a, b : integer) return integer is
begin
   if b = 0 then return a; else return gcd(b, a mod b); end if;
end gcd;
gcd(1071, 462)
pp(gcd'sym);
