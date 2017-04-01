:loop
set $c, -1
rm8 $a, $c
set $c, -2
wm8 $c, $a
set $a, @loop
jmp $a
