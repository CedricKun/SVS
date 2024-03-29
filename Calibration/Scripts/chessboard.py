import numpy as np
import cv2
import os
import threading
from os.path import join
from tqdm import tqdm
from Core.dataset import getFileList
from Core.file_utils import read_json, save_json
from Core.dataset import ImageFolder
colors_chessboard_bar = [
    [0, 0, 255],
    [0, 128, 255],
    [0, 200, 200],
    [0, 255, 0],
    [200, 200, 0],
    [255, 0, 0],
    [255, 0, 250]
]
def get_lines_chessboard(pattern=(9, 6)):
    w, h = pattern[0], pattern[1]
    lines = []
    lines_cols = []
    for i in range(w*h-1):
        lines.append([i, i+1])
        lines_cols.append(colors_chessboard_bar[(i//w)%len(colors_chessboard_bar)])
    return lines, lines_cols
def getChessboard3d(pattern, gridSize, axis='xy'):
    # 注意：这里为了让标定板z轴朝上，设定了短边是y，长边是x
    template = np.mgrid[0:pattern[0], 0:pattern[1]].T.reshape(-1,2)
    object_points = np.zeros((pattern[1]*pattern[0], 3), np.float32)
    # 长边是x,短边是z
    if axis == 'xz':
        object_points[:, 0] = template[:, 0]
        object_points[:, 2] = template[:, 1]
    elif axis == 'yx':
        object_points[:, 0] = template[:, 1]
        object_points[:, 1] = template[:, 0]
    elif axis == 'xy':
        object_points[:, 0] = template[:, 0]
        object_points[:, 1] = template[:, 1]
    else:
        raise NotImplementedError
    object_points = object_points * gridSize
    return object_points
def _findChessboardCorners(img, pattern, debug):
    "basic function"
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    retval, corners = cv2.findChessboardCorners(img, pattern, 
        flags=cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_FAST_CHECK + cv2.CALIB_CB_FILTER_QUADS)
    if not retval:
        return False, None
    corners = cv2.cornerSubPix(img, corners, (11, 11), (-1, -1), criteria)
    corners = corners.squeeze()
    return True, corners
def findChessboardCorners(img, annots, pattern, debug=False):
    conf = sum([v[2] for v in annots['keypoints2d']])
    if annots['visited'] and conf > 0:
        return True
    elif annots['visited']:
        return None
    annots['visited'] = True
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    # Find the chess board corners
    ret, corners = _findChessboardCorners(gray, pattern, debug)
    # found the corners
    show = img.copy()
    show = cv2.drawChessboardCorners(show, pattern, corners, ret)
    # assert corners.shape[0] == len(annots['keypoints2d'])
    # corners = np.hstack((corners, np.ones((corners.shape[0], 1))))
    # annots['keypoints2d'] = corners.tolist()
    if not corners is None:
        corners = np.hstack((corners, np.ones((corners.shape[0], 1))))
        annots['keypoints2d'] = corners.tolist()
    return show
def create_chessboard(path, image, pattern, gridSize, ext, overwrite=True):
    print('Create chessboard {}'.format(pattern))
    keypoints3d = getChessboard3d(pattern, gridSize=gridSize, axis=args.axis)
    keypoints2d = np.zeros((keypoints3d.shape[0], 3))
    imgnames = getFileList(join(path, image), ext=ext)
    template = {
        'keypoints3d': keypoints3d.tolist(),
        'keypoints2d': keypoints2d.tolist(),
        'pattern': pattern,
        'grid_size': gridSize,
        'visited': False
    }
    for imgname in tqdm(imgnames, desc='create template chessboard'):
        annname = imgname.replace(ext, '.json')
        annname = join(path, 'chessboard', annname)
        if os.path.exists(annname) and overwrite:
            # 覆盖keypoints3d
            data = read_json(annname)
            data['keypoints3d'] = template['keypoints3d']
            data['image_name'] = imgname
            save_json(annname, data)
        elif os.path.exists(annname) and not overwrite:
            continue
        else:
            save_json(annname, template)
def _detect_chessboard(datas, path, image, out, pattern):
    for imgname, annotname in datas:
        # imgname, annotname = dataset[i]
        # detect the 2d chessboard
        img = cv2.imread(imgname)
        annots = read_json(annotname)
        show = findChessboardCorners(img, annots, pattern)
        save_json(annotname, annots)
        outname = join(out, imgname.replace(path + '/{}/'.format(image), ''))
        os.makedirs(os.path.dirname(outname), exist_ok=True)
        if isinstance(show, np.ndarray):
            cv2.imwrite(outname, show)
def detect_chessboard(path, image, out, pattern, gridSize, args):
    create_chessboard(path, image, pattern, gridSize, ext=args.ext, overwrite=args.overwrite3d)
    dataset = ImageFolder(path, image=image, annot='chessboard', ext=args.ext)
    dataset.isTmp = False
    trange = list(range(len(dataset)))
    threads = []
    for i in range(args.mp):
        ranges = trange[i::args.mp]
        datas = [dataset[t] for t in ranges]
        thread = threading.Thread(target=_detect_chessboard, args=(datas, path, image, out, pattern)) # 应该不存在任何数据竞争
        thread.start()
        threads.append(thread)
    for thread in threads:
        thread.join()
if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', type=str)
    parser.add_argument('--image', type=str, default='images')
    parser.add_argument('--out', type=str, required=True)
    parser.add_argument('--ext', type=str, default='.jpg', choices=['.jpg', '.png'])
    parser.add_argument('--pattern', type=lambda x: (int(x.split(',')[0]), int(x.split(',')[1])),
        help='The pattern of the chessboard', default=(9, 6))
    parser.add_argument('--grid', type=float, default=0.1, 
        help='The length of the grid size (unit: meter)')
    parser.add_argument('--max_step', type=int, default=50)
    parser.add_argument('--min_step', type=int, default=0)
    parser.add_argument('--mp', type=int, default=4)
    parser.add_argument('--axis', type=str, default='xy')
    parser.add_argument('--silent', action='store_true')
    parser.add_argument('--debug', action='store_true')
    parser.add_argument('--overwrite3d', action='store_true')
    parser.add_argument('--seq', action='store_true')
    args = parser.parse_args()
    detect_chessboard(args.path, args.image, args.out, pattern=args.pattern, gridSize=args.grid, args=args)
