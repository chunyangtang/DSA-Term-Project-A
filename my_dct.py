import cv2
import numpy as np

BLOCK_SIZE = 8
THRESHOLD = 0
MD = 600011


def dct(img):
    img = np.array(img, dtype=np.float32)
    img_dct = cv2.dct(img)
    current_hash = 0
    for i in range(BLOCK_SIZE):
        for j in range(BLOCK_SIZE):
            if img_dct[i, j] > THRESHOLD:
                current_hash = (256 * current_hash + 1) % MD
            else:
                current_hash = (256 * current_hash) % MD

    return current_hash
