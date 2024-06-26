"""
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Manually select points to get the projection map
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"""
import argparse
import os
import numpy as np
import cv2
from Core import FisheyeCameraModel, PointSelector, display_image
import Core.params as settings


def get_projection_map(camera_model, image):
    und_image = camera_model.undistort(image)
    name = camera_model.camera_name
    gui = PointSelector(und_image, title=name)
    dst_points = settings.project_keypoints[name]
    choice = gui.loop()
    if choice > 0:
        src = np.float32(gui.keypoints)
        dst = np.float32(dst_points)
        camera_model.project_matrix = cv2.getPerspectiveTransform(src, dst)
        proj_image = camera_model.project(und_image)

        ret = display_image("Bird's View", proj_image)
        if ret > 0:
            return True
        if ret < 0:
            cv2.destroyAllWindows()

    return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-path", default=None,
                        help="The camera view to be projected")
    parser.add_argument("-camera", required=True,
                        choices=["front", "back", "left", "right"],
                        help="The camera view to be projected")
    parser.add_argument("-scale", nargs="+", default=None,
                        help="scale the undistorted image")
    parser.add_argument("-shift", nargs="+", default=None,
                        help="shift the undistorted image")
    parser.add_argument('-ext', type=str, default='.jpg', 
                        choices=['.jpg', '.png'])
    args = parser.parse_args()

    if args.scale is not None:
        scale = [float(x) for x in args.scale]
    else:
        scale = (0.4, 0.4)

    if args.shift is not None:
        shift = [float(x) for x in args.shift]
    else:
        shift = (0, 0)

    path = args.path if args.path else os.getcwd()
    camera_name = args.camera
    
    camera_file = os.path.join(path, "yaml", camera_name + ".yaml")
    image_file = os.path.join(path, "images", camera_name + args.ext)
    image = cv2.imread(image_file)
    camera = FisheyeCameraModel(camera_file, camera_name)
    camera.set_scale_and_shift(scale, shift)
    success = get_projection_map(camera, image)
    if success:
        print("saving projection matrix to yaml")
        camera.save_data()
    else:
        print("failed to compute the projection map")


if __name__ == "__main__":
    main()