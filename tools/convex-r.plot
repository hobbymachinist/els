# radius of arc circle
r = 12.5

# depth of arc intersecting stock
d = 5.0
# length of arc intersecting stock
l = 5.0

# stock
D = 25.0
L = 50.0

# circle origin calculation.
#
# see https://math.stackexchange.com/questions/1781438/finding-the-center-of-a-circle-given-two-points-and-a-radius-algebraically
#
# equation
# (x - a)^2 + (y - b)^2 = r^2
#
# intersects at 2 points.
#
# (0, -d) and (-l, 0)
#
# midpoint of rhombus
#
dx = l / 2.0
dy = d / 2.0
mx = -dx
my = -dy

# diagonal 1
d1 = sqrt(dx**2 + dy**2)
# diagonal 2
d2 = sqrt(r**2 - d1**2)

# circle origin
a = mx - (d2 * dy) / d1
b = my - (d2 * dx) / d1

print "l = ",l
print "d = ",d
print "radius = ",r

print "mx = ", mx
print "my = ", my

print "d1 = ", d1
print "d2 = ", d2

print "origin = (", a, ", ", b, ")"

set size ratio -1
set xrange [-L:L]
set yrange [D:-D] reverse

set label "L" at -l/2, -d-3
set arrow from -l, -d-1 to 0, -d-1 heads noborder lw 1 lc rgb "black"

set label "D" at -l-3.5, -d/2
set arrow from -l-1, -d to -l-1, 0 heads noborder lw 1 lc rgb "black"

set label "R" at a-2, b/2
set arrow from a, b to a, b + r heads noborder lw 1 lc rgb "black"

set arrow from -l,-d to 0,-d nohead lw 2 lc rgb "red"
set arrow from -l,-d to -l,0 nohead lw 2 lc rgb "red"

set arrow from 0,0 to 0,-D nohead lw 2 lc rgb "blue"
set arrow from -L,0 to 0,0 nohead lw 2 lc rgb "blue"
set arrow from -L,-D to 0,-D nohead lw 2 lc rgb "blue"

set object 1 circle front at a,b size r fillcolor rgb "gray" lw 2
plot 0 notitle lw 1
