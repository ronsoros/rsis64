comment The RSIS BIOS is licensed under the MIT license
comment Please see 'LICENSE.EMU' file.
comment -- Begin BIOS
set $a, @main
jmp $a
:strInfo
	ds RSIS-BIOS v1.0 (C) 2016, 2017 Ronsor-OpenStar
	db 10
	db 10
	ds Configured to boot from address starting at 6MiB
	db 10
	ds Press ESC for BIOS Options
	db 10
	db 0
:printstr
	set $a, 0
	set $b, 0
	set $d, -2
	set $e, -3
	set $f, -4
	set $g, 0
	set $h, 0
	:printloop
	rm8 $a, $c
	eq $a, $b
	set $t, @noloop
	jt $t
	set $n, 10
	eq $a, $n
	set $t, @newline
	jt $t
	set $t, @skip1
	jmp $t
	:newline
	set $t, @incry
	call $t
	set $t, 1
	add $c, $t

	set $t, @printloop
	jmp $t
	:skip1
	wm8 $d, $a
	set $t, @incrx
	call $t
	set $t, 1
	add $c, $t
	set $t, @printloop
	jmp $t
	:noloop
	ret
	:incrx
		set $s, 8
		add $g, $s
		wm64 $f, $g
		ret
	:incry
		set $s, 8
		add $h, $s
		wm64 $e, $h
		set $s, 0
		set $g, 0
		wm64 $f, $s
		ret
:main
	set $c, @strInfo
	set $a, @printstr
	call $a
	:abc
	set $a, @abc
	jmp $a
