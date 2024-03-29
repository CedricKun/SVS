import cv2
import numpy as np
from Core.file_utils import save_json
class extractor:
    def __init__(self, bev1, bev2, intri1, intri2, extri1, extri2):
        self.bev1 = bev1
        self.bev2 = bev2
        self.intri1 = intri1
        self.intri2 = intri2
        self.extri1 = extri1
        self.extri2 = extri2
        self.bin_bev = None
        self.contours = []
    def saveResult(self, filename, vec):
        save_json(filename, vec)
    def binarization(self):
        assert self.bev1.shape == self.bev2.shape
        mask = (self.bev1[:, :] != np.zeros((3, 1))) and (self.bev2[:, :] != np.zeros((3, 1)))
        bin_bev = np.ones(self.bev1.shape) *255
        bin_bev *= mask
        self.bin_bev = bin_bev
    def findContours(self):
        dilate_kernel = cv2.getStructuringElement(0, (5, 5))
        dilate_img = cv2.dilate(self.bin_bev, dilate_kernel)
        erode_kernel = cv2.getStructuringElement(0, (5, 5))
        erode_image = cv2.erode(self.bin_bev, erode_kernel)
        contours = cv2.findContours(erode_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
        maxsize, index = 0, 0
        for idx, con in enumerate(contours):
            if len(con) > maxsize:
                maxsize = len(con)
                index = idx
        contours_filtered = [contours[index]]
        contours_pixels = fillContour(contours_filtered)
        self.contours = contours_pixels
    def fillContour(self, _contours):
        c_pairs = []
        for idx, sub in enumerate(_contours):
            sub = sorted(sub, key=lambda s: (s[0], s[1]))
            _contours[idx] = sub
            pairs = []
            for c in sub:
                if not len(pairs):
                    pairs.append([c, c])
                    continue
                if c[0] != pairs[-1][0][0]:
                    pairs.append([c, c])
                    continue
                if c[0] == pairs[-1][0][0] and c[1] > pairs[-1][1][1]:
                    pairs[-1][1] = c
                    continue
            c_pairs.append(pairs)
        fill_con = []
        for pair in c_pairs:
            point_set = []
            for p in pair:
                if p[0] == p[1]: point_set.append(p[0])
                else:
                    for i in range(int(p[0][1]), int(p[1][1] + 1)):
                        point_set.append([p[0][0], i])
            fill_con.append(point_set)
        return fill_con
    def extractTexture(self, idx, size):
        down_sample = 500
        gray1 = cv2.cvtColor(self.bev1, cv2.COLOR_BGR2GRAY)
        gray1 = cv2.GaussianBlur(gray1, (3, 3), 0)
        gray2 = cv2.cvtColor(self.bev2, cv2.COLOR_BGR2GRAY)
        gray2 = cv2.GaussianBlur(gray2, (3, 3), 0)
        h, w = self.bev1.shape
        if (idx == "fl"):
            camera_center_x = w / 2;
            camera_center_y = size;
        elif (idx == "fr"):
            camera_center_x = w / 2;
            camera_center_y = size;
        elif (idx == "bl"):
            camera_center_x = w / 2;
            camera_center_y = h - size;
        elif (idx == "br"): 
            camera_center_x = w / 2;
            camera_center_y = h - size;
        elif (idx == "lf"):
            camera_center_x = size;
            camera_center_y = h / 2;
        elif (idx == "rf"):
            camera_center_x = w - size;
            camera_center_y = h / 2;
        elif (idx == "lb"):
            camera_center_x = size;
            camera_center_y = h / 2;
        elif (idx == "rb"):
            camera_center_x = w - size;
            camera_center_y = h / 2;
        common_view = []
        c_pixel = self.contours[0]
        for p in c_pixel:
            if p[0] < 10 or p[1] < 10 or p[0] > w - 10 or p[1] > h - 10: continue
            if abs(p[0] - camera_center_x) > 0.9 * size or abs(p[1] - camera_center_y) > 0.9 *size: continue
            p1 = [p[0] - 2, p[1]]
            p2 = [p[0], p[1] - 2]
            delta = np.array([gray1[p] - gray1[p1], gray1[p] - gray1[p2]])
            if np.linalg.norm(delta) < 15: continue
            common_view.append(p)
        common_view_d = []
        step = int(len(common_view) / down_sample)
        for i in range(0, len(common_view), step=step):
            common_view_d.append(common_view[i])
        return common_view_d[:down_sample] if len(common_view_d) > down_sample else common_view_d
