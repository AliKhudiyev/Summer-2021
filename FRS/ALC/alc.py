import numpy as np
import pandas as pd
import matplotlib
from matplotlib import pyplot as plt
from matplotlib import animation, patches
from matplotlib import path
import time
import random
from scipy import interpolate


print('Imma visualize the shit of this')

class Block:
    def __init__(self):
        pass

class Core:
    IO = 0
    SELECTOR_0 = 2
    SELECTOR_1 = 3
    NOT = 4

    def __init__(self, id_, name='', function=IO):
        self.id = id_
        self.function = function

        if function == Core.SELECTOR_0:
            self.name = '\u2227'
        elif function == Core.SELECTOR_1:
            self.name = '\u2228'
        elif function == Core.NOT:
            self.name = '\u00ac'
        else:
            self.name = name


class Interconnect:
    def __init__(self, id1, id2, support=1, speculative=False):
        self.ids = (id1, id2)
        self.support = support
        self.speculative = speculative

class Renderer:
    CORE_RADIUS = 0.5
    INTERCONNECT_WIDTH = 1
    VERTICAL_DIST = 3
    HORIZONTAL_DIST = 3

    FIGURE_INDEX = 0
    SYSTEM_RENDERER = 0
    STATS_RENDERER = 1

    def __init__(self, file_path, mode):
        self.fig = plt.figure(Renderer.FIGURE_INDEX, figsize=(7,6))
        if mode == Renderer.SYSTEM_RENDERER:
            self.fig.canvas.manager.set_window_title('ALC System')
            self.ax = self.fig.add_subplot(111)
            self.ax.axis('off')
        else:
            self.fig.canvas.manager.set_window_title('ALC Stats')
            self.ax = self.fig.add_subplot(211)
            self.ax2 = self.fig.add_subplot(212)
        self.fig.tight_layout()

        self.cores = []
        self.interconnects = []
        self.file_path = file_path
        self.prev_df = None
        self.mode = mode
        self.speed = 5
        self.anim_path = None

        Renderer.FIGURE_INDEX += 1

    def update_system(self, frame):
        # Process file
        # self.ax.patches = []
        df = pd.read_csv(self.file_path, skipinitialspace=True)
        if df.equals(self.prev_df):
            return
        self.prev_df = df
        self.ax.clear()
        layers = [[] for i in range((df['depth'].max() + 1))]
        interconnects = []
        coords_by_id = {}
        
        for row in df.values:
            index = row[5]
            if row[1] == 'core':
                if row[2] == 'input' or row[2] == 'output' or row[2] == 'io':
                    layers[index].append(Core(row[0], str(row[0]), function=Core.IO))
                elif row[2] == 'selector_0':
                    layers[index].append(Core(row[0], function=Core.SELECTOR_0))
                elif row[2] == 'selector_1':
                    layers[index].append(Core(row[0], function=Core.SELECTOR_1))
                else:
                    layers[index].append(Core(row[0], function=Core.NOT))
            else:
                specul = True if row[2] == 'speculative' else False
                interconnects.append(Interconnect(row[3], row[4], 
                    support=float(row[6]), speculative=specul))

        # Process objects
        for i, layer in enumerate(layers):
            center_layer = (0, 0)
            core_count = len(layer)
            r = Renderer.CORE_RADIUS
            style = "Simple, tail_width=0.5, head_width=4, head_length=8"
            kw = dict(arrowstyle=style, color="k")

            for j, core in enumerate(layer):
                patch = None
                center_core = (Renderer.HORIZONTAL_DIST*i, 
                        Renderer.VERTICAL_DIST*((core_count-1)/2-j))
                coords_by_id[core.id] = center_core
                if core.function == Core.IO:
                    patch = plt.Circle(center_core, r)
                else:
                    fc = 'red'
                    text = '\u00ac'
                    if core.function == Core.SELECTOR_0:
                        fc = 'orange'
                        text = '\u2227'
                    elif core.function == Core.SELECTOR_1:
                        fc = 'purple'
                        text = '\u2228'
                    patch = plt.Circle((center_core[0], center_core[1]), r, facecolor=fc)
                    self.ax.annotate(text, (center_core[0], center_core[1]), 
                            fontsize=15, color='black', ha='center', va='center')
                if patch is not None:
                    self.ax.add_patch(patch)

        for i, interconnect in enumerate(interconnects):
            id1, id2 = interconnect.ids
            center_core1 = coords_by_id[id1]
            center_core2 = coords_by_id[id2]
            layer1_index = center_core1[0] // Renderer.HORIZONTAL_DIST
            layer2_index = center_core2[0] // Renderer.HORIZONTAL_DIST

            if interconnect.speculative:
                style = 'simple'
            else:
                style = 'simple, tail_width=0.5, head_width=4, head_length=8'

            n_layers_between = layer2_index - layer1_index - 1
            tmp = 1 - interconnect.support
            if tmp == 1:
                tmp = 0
            kw = dict(arrowstyle=style, color=(tmp, tmp, tmp))
            arrow = None
            annot_points = [center_core1, center_core2]

            if n_layers_between == 0:
                arrow = patches.FancyArrowPatch((center_core1[0]+r, center_core1[1]), 
                        (center_core2[0]-r, center_core2[1]), **kw)
            else:
                points = [(center_core1[0]+r+0.05, center_core1[1])]
                curr_core_center = center_core1
                for i in range(layer1_index+1, layer2_index):
                    found_center = None
                    min_dist = np.inf
                    sign = 1
                    dy = 0

                    for core in layers[i]:
                        next_core_center = coords_by_id[core.id]
                        dist = (next_core_center[0]-curr_core_center[0])**2 + \
                            (next_core_center[1]-curr_core_center[1])**2
                        if min_dist > dist:
                            min_dist = dist
                            found_center = next_core_center
                            if next_core_center[1] > curr_core_center[1]:
                                sign = -1

                    tmp_dist = Renderer.HORIZONTAL_DIST**2 + (Renderer.VERTICAL_DIST)**2
                    if min_dist > tmp_dist:
                        dy = curr_core_center[1] - found_center[1] - sign * Renderer.VERTICAL_DIST

                    position = (
                            found_center[0],
                            found_center[1] + dy + sign * random.uniform(Renderer.CORE_RADIUS+0.5, 
                                                    Renderer.VERTICAL_DIST-Renderer.CORE_RADIUS-0.5)
                            )
                    curr_core_center = position
                    points.append(position)
                points.append((center_core2[0]-r-0.05, center_core2[1]))
                nodes = np.array(points)
                x = nodes[:,0]
                y = nodes[:,1]
                tck, u = interpolate.splprep([x,y], k=len(points)-1) # s=0
                x_new, y_new = interpolate.splev(np.linspace(0,1,4*len(points)+1), tck, der=0)
                annot_points[0] = (x_new[len(x_new)//2], y_new[len(y_new)//2])
                annot_points[1] = (x_new[len(x_new)//2+1], y_new[len(y_new)//2+1])

                self.ax.plot(x_new, y_new, color=(tmp, tmp, tmp))
                arrow = patches.FancyArrowPatch((x_new[-2], y_new[-2]), 
                        (x_new[-1], y_new[-1]), **kw)

            if interconnect.support > 0:
                self.ax.annotate(str(interconnect.support), 
                        ((annot_points[0][0]+annot_points[1][0])/2, 
                            (annot_points[1][1]+annot_points[1][1])/2+0.2),
                        fontsize=10, color='black', ha='center', va='center')
            self.ax.add_patch(arrow)
        self.ax.axis('scaled')
        self.ax.axis('off')
        return self.ax.patches

    def update_stats(self, frame):
        self.ax.clear()
        self.ax.set_ylim(0, 100)
        self.ax2.clear()
        self.ax2.set_ylim(0, 1)
        df = pd.read_csv(self.file_path, skipinitialspace=True)

        # Compression Stats
        self.ax.plot(df.loc[:, 'statically_compressed'].values)
        self.ax.plot(df.loc[:, 'dynamically_compressed'].values)
        self.ax.plot(df.loc[:, 'effectively_compressed'].values)
        self.ax.plot(df.loc[:, 'compressed'].values)
        self.ax.plot(df.loc[:, 'memory'].values)
        self.ax.legend(['statically compressed', 
            'dynamically compressed', 
            'effectively compressed', 
            'overall',
            'memory'], loc='upper right', fontsize='small')
        # self.ax.xlabel('#iter', fontsize='small')

        # System Stats
        self.ax2.plot(df.loc[:, 'learning_sensitivity'].values)
        self.ax2.plot(df.loc[:, 'aggressiveness'].values)
        self.ax2.plot(df.loc[:, 'tolerance'].values)
        self.ax2.plot(df.loc[:, 'lossiness'].values)
        self.ax2.plot(df.loc[:, 'overwhelm'].values)
        self.ax2.plot(df.loc[:, 'curiosity'].values)
        self.ax2.plot(df.loc[:, 'core_count'].values)
        self.ax2.plot(df.loc[:, 'interconnect_count'].values)
        self.ax2.legend(['data sensitivity',
            'aggressiveness',
            'tolerance',
            'lossiness',
            'overwhelm',
            'curiosity',
            'core count',
            'interconnect count'], loc='upper right', fontsize='small')
        # self.ax2.xlabel('#iter', fontsize='small')

    def update(self, frame):
        if self.mode == Renderer.SYSTEM_RENDERER:
            self.update_system(frame)
        else:
            self.update_stats(frame)

    def run(self):
        if self.speed:
            self.anim = animation.FuncAnimation(self.fig, self.update, 
                    interval=50+0*1000-(self.speed-5)*210, blit=False)
        else:
            self.update(0)

        # TODO: saving animation doesn't work, fix asap!
        # self.anim_path = 'system.mp4'
        if self.speed and self.anim_path is not None:
            self.anim.save(self.anim_path, writer=animation.FFMpegWriter(fps=1))
                    # extra_args=['-vcodec', 'h264', '-pix_fmt', 'yuv420p'])
        # plt.close()


renderer_system = Renderer('out.sys', Renderer.SYSTEM_RENDERER)
renderer_stats = Renderer('out.sta', Renderer.STATS_RENDERER)

renderer_system.run()
renderer_stats.run()

plt.show()
