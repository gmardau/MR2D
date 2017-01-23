reset

#set terminal tikz
#set output 'mesh.tex'
#set terminal qt size 1360, 730
set terminal pngcairo size 1920,1080
set output 'mesh.png'
set lmargin 0
set rmargin 0
set tmargin 0
set bmargin 0

set size ratio -1
unset border
unset key
unset xtics
unset ytics

#set xrange [-0.25:1.25]
#set yrange [-0.3:0.3]

set style line 1 lc rgb '#000000' lt 1 lw 0.75

plot 'mesh.dat' with lines ls 1

#pause -1
