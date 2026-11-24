-- Types
type day is (mon, tue, wed, thu, fri, sat, sun);
d : day := wed;
day'succ(d)
day'pos(sun)
subtype weekday is day range mon..fri;
for x in weekday loop putint(day'pos(x)); end loop;
new_line;
type matrix is array(1..2, 1..2) of integer;
m : matrix;
m(1,1) := 1; m(1,2) := 2; m(2,1) := 3; m(2,2) := 4;
m(1,1) * m(2,2) - m(1,2) * m(2,1)
integer'last
dir;
