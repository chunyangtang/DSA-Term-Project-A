import cv2
import numpy as np
import os
from tqdm import tqdm

QUERY_SIZE = 64

# Please change the output path of main_dct_pc.cpp simultaneous
i_path = 'test_folder_advanced/test'
o_path = 'process_folder.nosync'

if __name__ == '__main__':
    if os.path.isdir(i_path):
        for f_name in os.listdir(i_path):
            if f_name.__len__() >= 6 and f_name[-6:] == '_g.png':
                img = cv2.imread(os.path.join(i_path, f_name),
                                 cv2.IMREAD_GRAYSCALE).astype(int)

                if f_name == 'source_g.png':
                    source = img
                    for i in tqdm(range(source.shape[0] - QUERY_SIZE+1)):
                        for j in range(source.shape[1] - QUERY_SIZE+1):
                            img = source[i:i + QUERY_SIZE, j:j + QUERY_SIZE]

                            img = np.array(img, dtype=np.float32)
                            img = cv2.dct(img)
                            img_average = np.median(img)
                            img = np.where(img > img_average, 1, 0)

                            h, w = img.shape[0], img.shape[1]
                            i_str = str(i)
                            f = open(os.path.join(
                                o_path, f_name[:-4] + str(i) + '_' + str(j) + '.data'), 'wb')
                            f.write(h.to_bytes(4, 'little'))
                            f.write(w.to_bytes(4, 'little'))
                            for r in range(h):
                                for c in range(w):
                                    t = int(img[r, c])
                                    f.write(t.to_bytes(4, 'little'))
                            f.close()

                else:
                    img = np.array(img, dtype=np.float32)
                    img = cv2.dct(img)
                    img_average = np.median(img)
                    img = np.where(img > img_average, 1, 0)

                    h, w = img.shape[0], img.shape[1]
                    f = open(os.path.join(o_path, f_name[:-4] + '.data'), 'wb')
                    f.write(h.to_bytes(4, 'little'))
                    f.write(w.to_bytes(4, 'little'))
                    for r in range(h):
                        for c in range(w):
                            t = int(img[r, c])
                            f.write(t.to_bytes(4, 'little'))
                    f.close()
