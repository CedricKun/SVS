import os
import numpy as np
import cv2
import argparse
from PIL import Image
from Core import FisheyeCameraModel, display_image, BirdView
import Core.params as settings


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-path", type=str, default=None,
                        help="The camera view to be projected")
    parser.add_argument('-ext', type=str, default='.jpg', 
                        choices=['.jpg', '.png'])
    args = parser.parse_args()

    path = args.path if args.path else os.getcwd()
    names = settings.camera_names
    
    images = [os.path.join(path, "images", name + args.ext) for name in names]
    yamls = [os.path.join(path, "yaml", name + ".yaml") for name in names]
    camera_models = [FisheyeCameraModel(camera_file, camera_name) for camera_file, camera_name in zip (yamls, names)]

    projected = []
    for image_file, camera in zip(images, camera_models):
        view = image_file.split('\\')[-1].split('.')[0]
        img = cv2.imread(image_file)
        img = camera.undistort(img)
        # cv2.imwrite('\\'.join(['res', f'undistort_{view}.png']), img)
        img = camera.project(img)
        # cv2.imwrite('\\'.join(['res', f'project_{view}.png']), img)
        img = camera.flip(img)
        # cv2.imwrite('\\'.join(['res', f'flip_{view}.png']), img)
        projected.append(img)

    birdview = BirdView()
    Gmat, Mmat = birdview.get_weights_and_masks(projected)
    birdview.update_frames(projected)
    birdview.make_luminance_balance().stitch_all_parts()
    birdview.make_white_balance()
    birdview.copy_car_image()
    ret = display_image("BirdView Result", birdview.image)
    cv2.imwrite('bev.png', birdview.image)
    if ret > 0:
        Image.fromarray((Gmat * 255).astype(np.uint8)).save("weights.png")
        Image.fromarray(Mmat.astype(np.uint8)).save("masks.png")


if __name__ == "__main__":

    main()