import numpy as np
from matplotlib import pyplot as plt
import copy
import sys


def make_line(line1, line2, proximity):
    line = [[], []]
    p1, p2, p3 = line1[0], line1[1], line2[1]
    line[0] = [p1[0]+proximity*(p2[0]-p1[0]), p1[1]+proximity*(p2[1]-p1[1])]
    line[1] = [p3[0]+proximity*(p2[0]-p3[0]), p3[1]+proximity*(p2[1]-p3[1])]
    return line

def bend(line1, line2, degree, proximity):
    points = []
    line = make_line(line1, line2, proximity)
    line1_new = [line1[0], line[0]]
    line2_new = [line[1], line2[1]]
    center = [(line[0][0]+line[1][0])/2, (line[0][1]+line[1][1])/2]
    lines = [[line[0], center], [center, line[1]]]

    if degree > 1:
        points1 = bend(line1_new, lines[0], degree-1, proximity)
        points2 = bend(lines[1], line2_new, degree-1, proximity)
        points = points1 + points2
    elif degree == 1:
        points.append([line[0][0], line[0][1]])
        points.append([line[1][0], line[1][1]])
    else:
        points = [line1[0], line1[1], line2[1]]

    return points


points = [[0, 0], [1, 4], [2, 0], [4, 3], [1.5, 2.5]]
curvature_degree = int(sys.argv[1])
curvature_proximity = 0.5
xs, ys = [points[0][0]], [points[0][1]]

prev_point = points[0]
for i in range(2, len(points)):
    mid_point = [(points[i-1][0]+points[i][0])/2, (points[i-1][1]+points[i][1])/2]
    if i + 1 == len(points):
        mid_point = points[-1]
    line1 = [prev_point, points[i-1]]
    line2 = [points[i-1], mid_point]
    prev_point = mid_point
    pts = bend(line1, line2, curvature_degree, curvature_proximity)
    for p in pts:
        xs.append(p[0])
        ys.append(p[1])

xs.append(points[-1][0])
ys.append(points[-1][1])

plt.plot(xs, ys, '-')
plt.scatter(np.array(points)[:,0], np.array(points)[:,1])
plt.axis('scaled')
plt.show()
