:test
set $n, 1
set $a, 24576
set $b, 0xFF
:loop
wm8 $a, $b
add $a, $n
wm8 $a, $b
add $a, $n
add $a, $n
wm8 $a, $b
set $c, @loop
add $a, $n
jmp $c
set $a, @test
set $w, 55
jmp $a
hlt
