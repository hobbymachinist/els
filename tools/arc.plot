# attrs
r = 12.5
d = 5.0
l = sqrt((r*r) - ((r-d)*(r-d)))

# stock
D = 25.0
L = 100.0

print "radius = ",r
print "l = ",l
print "d = ",d

set arrow from 0,-r to 0,-r+d nohead lw 2 lc rgb "red"
set arrow from 0,-r+d to l,-r+d nohead lw 2 lc rgb "red"

set arrow from l-L,-r to l,-r nohead lw 3 lc rgb "blue"
set arrow from l-L,-r+D to l,-r+D nohead lw 3 lc rgb "blue"
set arrow from l,-r to l,-r+D nohead lw 3 lc rgb "blue"

set size ratio -1
f(x) = sqrt(r**2 - x**2)
plot [-r:r][-r:r] f(x) notitle lw 2 lc rgb "gray", -1*f(x) notitle lw 2 lc rgb "gray"
