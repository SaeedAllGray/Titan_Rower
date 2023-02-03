from numpy import array, zeros
from rrtplanner import RRTStar, random_point_og
from math import sqrt
import pickle

class PathPlanner:
    DIM_ORI = 480 # dimension of original image
    SCALE_DOWN = 2 # scale down for the RTT* algorithm
    DIM_ALG = int(DIM_ORI / SCALE_DOWN)

    # 1300 mm (real life) -> 480 (image dimension) -> 240 (algorithm)
    SCALE_RL = DIM_ALG / 1300
    # 200 * 100 mm (RL)
    # 250 * 125 mm (scenario1.jpg)
    OBS_W_DIV2 = int(200 * SCALE_RL / 2)
    OBS_H_DIV2 = int(100 * SCALE_RL / 2)
    MARGIN = 5
    ROVER_RADIUS = int((93 / sqrt(2) + MARGIN) * SCALE_RL)

    # RTT* specific
    n = 1200
    r_rewire = 80  # large enough for our 400x400 world

    def __init__(self, xstart, xgoal, obstacles_HV_scenario, start_point):
        
        # process args: scale them down

        self.xstart = array([int(xstart[0] / self.SCALE_DOWN), int(xstart[1] / self.SCALE_DOWN)])
        self.xgoal = array([int(xgoal[0] / self.SCALE_DOWN), int(xgoal[1] / self.SCALE_DOWN)])
        self.obstacles_HV_scenario = [(int(i[0] / self.SCALE_DOWN), int(i[1] / self.SCALE_DOWN), i[2]) for i in obstacles_HV_scenario]

        # set-up the scenario
        self.og = zeros((self.DIM_ALG, self.DIM_ALG), dtype = int)

        # draw obstacles on og
        for p in self.obstacles_HV_scenario:
            if p[2] == "obstacleH":
                wid = self.OBS_W_DIV2 + self.ROVER_RADIUS
                hei = self.OBS_H_DIV2 + self.ROVER_RADIUS
                # for calculating rounded rectangle
                top_left = ((p[0] - self.OBS_H_DIV2, p[1] - self.OBS_W_DIV2))
                top_right = ((p[0] + self.OBS_H_DIV2, p[1] - self.OBS_W_DIV2))
                bot_left = ((p[0] - self.OBS_H_DIV2, p[1] + self.OBS_W_DIV2))
                bot_right = ((p[0] + self.OBS_H_DIV2, p[1] + self.OBS_W_DIV2))
            else:
                wid = self.OBS_H_DIV2 + self.ROVER_RADIUS
                hei = self.OBS_W_DIV2 + self.ROVER_RADIUS
                # for calculating rounded rectangle
                top_left = ((p[0] - self.OBS_W_DIV2, p[1] - self.OBS_H_DIV2))
                top_right = ((p[0] + self.OBS_W_DIV2, p[1] - self.OBS_H_DIV2))
                bot_left = ((p[0] - self.OBS_W_DIV2, p[1] + self.OBS_H_DIV2))
                bot_right = ((p[0] + self.OBS_W_DIV2, p[1] + self.OBS_H_DIV2))

            for y in range(p[1] - wid, p[1] + wid):
                if y < 0 or y >= self.DIM_ALG:
                    continue
                for x in range(p[0] - hei, p[0] + hei):
                    # skip if out of image boundaries
                    if x < 0 or x >= self.DIM_ALG:
                        continue

                    # skip if out of the boundary of rounded corners:
                    if (x < top_left[0] and y < top_left[1]):
                        if sqrt((top_left[0] - x) ** 2 + (top_left[1] - y) ** 2) > self.ROVER_RADIUS:
                            continue
                    if (x > top_right[0] and y < top_right[1]):
                        if sqrt((top_right[0] - x) ** 2 + (top_right[1] - y) ** 2) > self.ROVER_RADIUS:
                            continue
                    if (x < bot_left[0] and y > bot_left[1]):
                        if sqrt((bot_left[0] - x) ** 2 + (bot_left[1] - y) ** 2) > self.ROVER_RADIUS:
                            continue
                    if (x > bot_right[0] and y > bot_right[1]):
                        if sqrt((bot_right[0] - x) ** 2 + (bot_right[1] - y) ** 2) > self.ROVER_RADIUS:
                            continue

                    self.og[x][y] = 1

    def calculate(self):
        # if start / end at a inaccessable point, try neighbour points:
        if self.og[self.xstart[0]][self.xstart[1]] == 1:
            origin_pt = self.xstart
            i = 1
            pt_found = False
            while(not pt_found):
                # try in 4 directions
                try_pts = [(-i, 0), (i, 0), (0, -i), (0, i)]
                for pt in try_pts:
                    new_pt = array([origin_pt[0] + pt[0], origin_pt[1] + pt[1]])
                    if new_pt[0] < 0 or new_pt[0] >= self.DIM_ALG or new_pt[1] < 0 or new_pt[1] >= self.DIM_ALG:
                        continue
                    if self.og[new_pt[0]][new_pt[1]] == 0:
                        pt_found = True
                        break
                i = i + 1
            self.xstart = new_pt
            print("Start: {} -> {}".format(origin_pt, new_pt))

        if self.og[self.xgoal[0]][self.xgoal[1]] == 1:
            origin_pt = self.xgoal
            i = 1
            pt_found = False
            while(not pt_found):
                # try in 4 directions
                try_pts = [(-i, 0), (i, 0), (0, -i), (0, i)]
                for pt in try_pts:
                    new_pt = array([origin_pt[0] + pt[0], origin_pt[1] + pt[1]])
                    if new_pt[0] < 0 or new_pt[0] >= self.DIM_ALG or new_pt[1] < 0 or new_pt[1] >= self.DIM_ALG:
                        continue
                    if self.og[new_pt[0]][new_pt[1]] == 0:
                        pt_found = True
                        break
                i = i + 1
            self.xgoal = new_pt
            print("Goal: {} -> {}".format(origin_pt, new_pt))

        # start RRT*
        rrts = RRTStar(self.og, self.n, self.r_rewire)

        self.T, gv = rrts.plan(self.xstart, self.xgoal)

        path = rrts.route2gv(self.T, gv)
        path_pts = rrts.vertices_as_ndarray(self.T, path)

        # path_scaled: simple array & is scaled up by SCALE_DOWN
        path_scaled = [(self.xstart[0] * self.SCALE_DOWN, self.xstart[1] * self.SCALE_DOWN)] + [(i[1][0] * self.SCALE_DOWN, i[1][1] * self.SCALE_DOWN) for i in path_pts]
        return path_pts, path_scaled

if __name__ == "__main__":

    target_list = []
    obstacles_HV_scenario = []
    start_point = []
    with open('target_list.pickle', 'rb') as file:
        target_list = pickle.load(file)
    with open('obstacles_HV_scenario.pickle', 'rb') as file:
        obstacles_HV_scenario = pickle.load(file)
    with open('start_point.pickle', 'rb') as file:
        start_point = pickle.load(file)

    target_paths = []
    for i in range(len(target_list)):
        if i == 0:
            xstart = array([start_point[0][0], start_point[0][1]])
        else:
            xstart = array([target_list[i - 1][0][0], target_list[i - 1][0][1]])
        xgoal = array([target_list[i][0][0], target_list[i][0][1]])

        # xstart = array([start_point[0][0], start_point[0][1]])
        # xgoal = array([target_list[0][0][0], target_list[0][0][1]])
        pp = PathPlanner(xstart, xgoal, obstacles_HV_scenario, start_point)
        p_pts, path_scaled = pp.calculate()
        target_paths.append(path_scaled)

        from rrtplanner import plot_rrt_lines, plot_path, plot_og, plot_start_goal
        import matplotlib.pyplot as plt

        # # create figure and ax.
        # fig = plt.figure()
        # ax = fig.add_subplot()

        # # these functions alter ax in-place.
        # plot_og(ax, pp.og)
        # plot_start_goal(ax, pp.xstart, pp.xgoal)
        # print(ax)
        # print(pp.T)
        # plot_rrt_lines(ax, pp.T)
        # plot_path(ax, p_pts)

        # plt.show()

    with open('target_paths.pickle', 'wb') as file:
        pickle.dump(target_paths, file)
