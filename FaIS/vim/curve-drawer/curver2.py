import numpy as np
from matplotlib import pyplot as plt
import sys


def determine_next_point(curr_point, original_points, step):
    next_point = [0, 0]
    return next_point


points = [[0, 0], [1, 4], [2, 0], [4, 3], [1.5, 2.5]]
curvature_degree = int(sys.argv[1])
curvature_proximity = 0.5
step = 0.1
xs, ys = [points[0][0]], [points[0][1]]

curr_point = points[0]
while curr_point != points[-1]:
    curr_point = determine_next_point(curr_point, points, step)
    xs.append(curr_point[0])
    ys.append(curr_point[1])

plt.plot(xs, ys, '-')
plt.scatter(np.array(points)[:,0], np.array(points)[:,1])
plt.axis('scaled')
plt.show()
