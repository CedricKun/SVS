import os
import cv2


camera_names = ["front", "back", "left", "right"]

# --------------------------------------------------------------------
# (shift_width, shift_height): how far away the birdview looks outside
# of the calibration pattern in horizontal and vertical directions
shift_w = 350
shift_h = 350

# size of the gap between the calibration pattern and the car
# in horizontal and vertical directions
inn_shift_w = -15
inn_shift_h = 25

# total width/height of the stitched image
total_w = 610 + 2 * shift_w
total_h = 830 + 2 * shift_h

# four corners of the rectangular region occupied by the car
# top-left (x_left, y_top), bottom-right (x_right, y_bottom)
xl = shift_w + 150 + inn_shift_w
xr = total_w - xl
yt = shift_h + 150 + inn_shift_h
yb = total_h - yt
# --------------------------------------------------------------------

project_shapes = {
    "front": (total_w, yt),
    "back":  (total_w, yt),
    "left":  (total_h, xl),
    "right": (total_h, xl)
}

# pixel locations of the four points to be chosen.
# you must click these pixels in the same order when running
# the get_projection_map.py script
project_keypoints = {
    "front": [(shift_w + 225, shift_h + 25),
              (shift_w + 365, shift_h + 25),
              (shift_w + 225, shift_h + 125),
              (shift_w + 365, shift_h + 125)],

    "back":  [(shift_w + 245, shift_h + 25),
              (shift_w + 385, shift_h + 25),
              (shift_w + 245, shift_h + 125),
              (shift_w + 385, shift_h + 125)],

    "left":  [(shift_h + 445, shift_w + 25),
              (shift_h + 585, shift_w + 25),
              (shift_h + 445, shift_w + 125),
              (shift_h + 585, shift_w + 125)],

    "right": [(shift_h + 245, shift_w + 25),
              (shift_h + 385, shift_w + 25),
              (shift_h + 245, shift_w + 125),
              (shift_h + 385, shift_w + 125)]
}

car_image = cv2.imread(os.path.join(os.getcwd(), "images", "car.png"))
car_image = cv2.resize(car_image, (xr - xl, yb - yt))
# car_image = cv2.resize(car_image, (1, 1))