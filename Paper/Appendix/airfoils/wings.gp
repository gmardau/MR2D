reset

set size ratio -1
unset border
unset key
unset xtics
unset ytics

set xrange [-0.1:1.1]

set style line 1 lc rgb '#000000' lw 2
set style line 2 lc rgb '#888888' lw 2

list = system('ls *.dat')

do for [file in list] {
	set terminal tikz
	set output '../figures/airfoils/'.file[:strlen(file)-4].'.tex'
	plot file index 0 with lines ls 2, file index 1 with lines ls 1
}
