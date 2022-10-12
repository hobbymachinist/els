# config
set angles degrees

# diameter
D = 13

# length
L = 30

# arc len
X = (D / 2.0) * pi

# helix angle
A = 25

# pitch
P = X / tan((90 - A))

print "D = ", D
print "A = ", A
print "P = ", P

set object 1 rect from 0,0 to L,D fc rgb "grey"

do for [t=0:L] {
  set arrow from t*2,0 to P+t*2,D nohead lw 1 lc rgb "black"
  set arrow from t*2,D to P+t*2,0 nohead lw 1 lc rgb "black"
}

do for [t=-L:0] {
  set arrow from t*2,0 to P+t*2,D nohead lw 1 lc rgb "black"
  set arrow from t*2,D to P+t*2,0 nohead lw 1 lc rgb "black"
}

plot [0:L][-5:D+5] 0 notitle lw 1
