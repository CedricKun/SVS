import shutil
import random
import numpy as np
import cv2
import os
from os.path import join
from glob import glob
from tqdm import tqdm
from Scripts.chessboard import get_lines_chessboard
from Core.visualization import plot_points2d
from Core.file_utils import read_json, save_json, write_intri, save_result
from Core.utils import Timer

def read_chess(chessname):
    data = read_json(chessname)
    k3d = np.array(data['keypoints3d'], dtype=np.float32)
    k2d = np.array(data['keypoints2d'], dtype=np.float32)
    if (k2d[:, -1] > 0.).sum() < k2d.shape[0]//2:
        return False, k2d, k3d
    if k2d[:, -1].sum() < k2d.shape[0]:
        valid = k2d[:, -1] > 0.1
        k2d = k2d[valid]
        k3d = k3d[valid]
    return True, k2d, k3d

def pop(k2ds_, k3ds_, valid_idx, imgnames, max_num):
    k2ds = np.stack(k2ds_)
    dist = np.linalg.norm(k2ds[:, None] - k2ds[None, :], axis=-1).mean(axis=-1)
    size = np.linalg.norm(k2ds[:, -1] - k2ds[:, 0], axis=-1)
    dist = dist / size[:, None]
    row = np.arange(dist.shape[0])
    dist[row, row] = 9999.
    col = dist.argmin(axis=0)
    dist_min = dist[row, col]
    indices = dist_min.argsort()[:dist_min.shape[0] - max_num]
    if False:
        img0 = cv2.imread(imgnames[valid_idx[idx]])
        img1 = cv2.imread(imgnames[valid_idx[remove_id]])
        cv2.imshow('01', np.hstack([img0, img1]))
        cv2.waitKey(10)
        print('remove: ', imgnames[valid_idx[remove_id]], imgnames[valid_idx[idx]])
    indices = indices.tolist()
    indices.sort(reverse=True, key=lambda x:col[x])
    removed = set()
    for idx in indices:
        remove_id = col[idx]
        if remove_id in removed:
            continue
        removed.add(remove_id)
        valid_idx.pop(remove_id)
        k2ds_.pop(remove_id)
        k3ds_.pop(remove_id)

def load_chessboards(chessnames, imagenames, max_image, sample_image=-1, out='debug-calib'):
    os.makedirs(out, exist_ok=True)
    k3ds_, k2ds_, imgs = [], [], []
    valid_idx = []
    for i, chessname in enumerate(tqdm(chessnames, desc='read')):
        flag, k2d, k3d = read_chess(chessname)
        if not flag:
            continue
        k3ds_.append(k3d[:, None, :])
        k2ds_.append(k2d[:, None, :-1])
        valid_idx.append(i)
        if max_image > 0 and len(valid_idx) > max_image + int(max_image * 0.1):
            pop(k2ds_, k3ds_, valid_idx, imagenames, max_num=max_image)
    if sample_image > 0:
        print('[calibration] Load {} images, sample {} images'.format(len(k3ds_), sample_image))
        index = [i for i in range(len(k2ds_))]
        index_sample = random.sample(index, min(sample_image, len(index)))
        valid_idx = [valid_idx[i] for i in index_sample]
        k2ds_ = [k2ds_[i] for i in index_sample]
        k3ds_ = [k3ds_[i] for i in index_sample]
    else:
        print('[calibration] Load {} images'.format(len(k3ds_)))
    # for ii, idx in enumerate(valid_idx):
    #     shutil.copyfile(imagenames[idx], join(out, '{:06d}.jpg'.format(ii)))
    return k3ds_, k2ds_

def calib_intri_share(path, image, ext):
    camnames = sorted(os.listdir(join(path, image)))
    camnames = [cam for cam in camnames if os.path.isdir(join(path, image, cam))]
    imagenames = sorted(glob(join(path, image, '*', '*' + ext)))
    chessnames = sorted(glob(join(path, 'chessboard', '*', '*.json')))
    k3ds_, k2ds_ = load_chessboards(chessnames, imagenames, args.num, args.sample, out=join(args.path, 'output'))
    with Timer('calibrate'):
        print('[Info] start calibration with {} detections'.format(len(k2ds_)))
        gray = cv2.imread(imagenames[0], 0)
        k3ds = k3ds_
        k2ds = [np.ascontiguousarray(k2d[:, :-1]) for k2d in k2ds_]
        resolution = gray.shape[::-1]
        ret, K, dist, rvecs, tvecs = cv2.fisheye.calibrate(
            k3ds, k2ds, resolution, None, None, flags=cv2.fisheye.CALIB_FIX_SKEW|cv2.fisheye.CALIB_RECOMPUTE_EXTRINSIC,
            criteria=(cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_COUNT, 30, 1e-6))
        cameras = {}
        for cam in camnames:
            cameras = {
                'camera_matrix': K,
                'dist_coeffs': dist,  # dist: (1, 4)
                'resolution': np.int32(resolution)
            }
            save_result(join(path, 'yaml_share', f'{cam}.yaml'), cameras)

def calib_intri(path, image, ext):
    camnames = sorted(os.listdir(join(path, image)))
    camnames = [cam for cam in camnames if os.path.isdir(join(path, image, cam))]
    for ic, cam in enumerate(camnames):
        cameras = {}
        imagenames = sorted(glob(join(path, image, cam, '*'+ext)))
        chessnames = sorted(glob(join(path, 'chessboard', cam, '*.json')))
        k3ds_, k2ds_ = load_chessboards(chessnames, imagenames, args.num, out=join(args.path, 'output', cam+'_used'))
        k3ds = k3ds_
        k2ds = [np.ascontiguousarray(k2d) for k2d in k2ds_]
        gray = cv2.imread(imagenames[0], 0)
        resolution = gray.shape[::-1]
        print('>> Camera {}: {:3d} frames'.format(cam, len(k2ds)))
        with Timer('calibrate'):
            ret, K, dist, rvecs, tvecs = cv2.fisheye.calibrate(
                k3ds, k2ds, resolution, None, None, flags=cv2.fisheye.CALIB_FIX_SKEW|cv2.fisheye.CALIB_RECOMPUTE_EXTRINSIC,
            criteria=(cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_COUNT, 30, 1e-6))
            cameras = {
                'camera_matrix': K,
                'dist_coeffs': dist,  # dist: (1, 4)
                'resolution': np.int32(resolution)
            }
        save_result(join(path, 'yaml', f'{cam}.yaml'), cameras)
        reproj_err = []
        for i in range(len(k3ds)):
            corners_reproj, _ = cv2.fisheye.projectPoints(k3ds[i], rvecs[i], tvecs[i], K, dist)
            err = cv2.norm(corners_reproj, k2ds[i], cv2.NORM_L2) / len(corners_reproj)
            reproj_err.append(err)
        print(np.average(reproj_err))


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', type=str, default='/home/')
    parser.add_argument('--image', type=str, default='images')
    parser.add_argument('--ext', type=str, default='.jpg', choices=['.jpg', '.png'])
    parser.add_argument('--num', type=int, default=-1)
    parser.add_argument('--sample', type=int, default=-1)
    parser.add_argument('--share_intri', action='store_true')
    parser.add_argument('--remove', action='store_true')
    args = parser.parse_args()
    if args.share_intri:
        calib_intri_share(path, image, ext)
    else:
        calib_intri(args.path, args.image, ext=args.ext)
