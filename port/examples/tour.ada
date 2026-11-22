-- a short tour.  A line that is an expression (no ';') prints its value;
-- a line ending in ';' is a declaration or statement.
x := 3 + 4;
x * 6
type color is (red, green, blue);
c : color := blue;
c
type vec is array(1..3) of integer;
v : vec := (1, 2, 3);
v(2) := 20;
v(1) + v(2) + v(3)
2.5 * 4.0
s : string(1..5) := "hello";
put_line(s & " world");
procedure bump(a : in out integer) is begin a := a + 1; end bump;
y : integer := 41;
bump(y);
y
my_err : exception;
begin raise my_err; exception when my_err => put_line("caught it"); end;
package counter is n : integer := 0; procedure tick; end counter;
package body counter is procedure tick is begin n := n + 1; end tick; end counter;
counter.tick; counter.tick;
counter.n
for i in 1..5 loop putint(i * i); putstr(" "); end loop;
new_line;
pp(bump'sym);
dir;
