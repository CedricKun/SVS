import cv2
import numpy as np
import os
import json
from os.path import join
class FileStorage(object):
    def __init__(self, filename, isWrite=False):
        version = cv2.__version__
        self.major_version = int(version.split('.')[0])
        self.second_version = int(version.split('.')[1])
        if isWrite:
            os.makedirs(os.path.dirname(filename), exist_ok=True)
            self.fs = open(filename, 'w')
            self.fs.write('%YAML:1.0\r\n')
            self.fs.write('---\r\n')
        else:
            assert os.path.exists(filename), filename
            self.fs = cv2.FileStorage(filename, cv2.FILE_STORAGE_READ)
        self.isWrite = isWrite
    def __del__(self):
        if self.isWrite:
            self.fs.close()
        else:
            cv2.FileStorage.release(self.fs)
    def _write(self, out):
        self.fs.write(out+'\r\n')
    def write(self, key, value, dt='mat'):
        if dt == 'mat':
            self._write('{}: !!opencv-matrix'.format(key))
            self._write('  rows: {}'.format(value.shape[0]))
            self._write('  cols: {}'.format(value.shape[1]))
            self._write('  dt: d')
            self._write('  data: [{}]'.format(', '.join(['{:.6f}'.format(i) for i in value.reshape(-1)])))
        elif dt == 'list':
            self._write('{}:'.format(key))
            for elem in value:
                self._write('  - "{}"'.format(elem))
        elif dt == 'int':
            self._write('{}: {}'.format(key, value))
    def read(self, key, dt='mat'):
        if dt == 'mat':
            output = self.fs.getNode(key).mat()
        elif dt == 'list':
            results = []
            n = self.fs.getNode(key)
            for i in range(n.size()):
                val = n.at(i).string()
                if val == '':
                    val = str(int(n.at(i).real()))
                if val != 'none':
                    results.append(val)
            output = results
        elif dt == 'int':
            output = int(self.fs.getNode(key).real())
        else:
            raise NotImplementedError
        return output
    def close(self):
        self.__del__(self)
def read_intri(intri_name):
    assert os.path.exists(intri_name), intri_name
    intri = FileStorage(intri_name)
    camnames = intri.read('names', dt='list')
    cameras = {}
    for key in camnames:
        cam = {}
        cam['K'] = intri.read('K_{}'.format(key))
        cam['invK'] = np.linalg.inv(cam['K'])
        cam['dist'] = intri.read('dist_{}'.format(key))
        cameras[key] = cam
    return cameras, camnames
def write_intri(intri_name, cameras):
    if not os.path.exists(os.path.dirname(intri_name)):
        os.makedirs(os.path.dirname(intri_name))
    intri = FileStorage(intri_name, True)
    results = {}
    camnames = list(cameras.keys())
    intri.write('names', camnames, 'list')
    for key, val in cameras.items():
        K, dist = val['K'], val['dist']
        assert K.shape == (3, 3), K.shape
        assert dist.shape == (1, 5) or dist.shape == (5, 1) or dist.shape == (1, 4) or dist.shape == (4, 1), dist.shape
        intri.write('K_{}'.format(key), K)
        intri.write('dist_{}'.format(key), dist.flatten()[None])
def read_extri(extri_name):
    assert os.path.exists(extri_name), extri_name
    extri = FileStorage(extri_name)
    camnames = extri.read('names', dt='list')
    cameras = {}
    for key in camnames:
        cam = {}
        cam['T'] = extri.read('T_{}'.format(key))
        cameras[key] = cam
    return cameras, camnames
def write_extri(extri_name, cameras):
    if not os.path.exists(os.path.dirname(extri_name)):
        os.makedirs(os.path.dirname(extri_name))
    extri = FileStorage(extri_name, True)
    results = {}
    camnames = list(cameras.keys())
    extri.write('names', camnames, 'list')
    for key_, val in cameras.items():
        assert T.shape == (4, 4), T.shape
        key = key_.split('.')[0]
        extri.write('T_{}'.format(key), val['T'])
def read_json(path):
    assert os.path.exists(path), path
    with open(path) as f:
        try:
            data = json.load(f)
        except:
            print('Reading error {}'.format(path))
            data = []
    return data
def save_json(file, data):
    if file is None:
        return 0
    if not os.path.exists(os.path.dirname(file)):
        os.makedirs(os.path.dirname(file))
    with open(file, 'w') as f:
        json.dump(data, f, indent=4)
def save_result(file, data):
    if not os.path.exists(os.path.dirname(file)):
        os.makedirs(os.path.dirname(file))
    intri = cv2.FileStorage(file, cv2.FILE_STORAGE_WRITE)
    for key, val in data.items():
        intri.write(key, val)
    intri.release()
