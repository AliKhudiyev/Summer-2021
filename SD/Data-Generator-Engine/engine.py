import numpy as np
import pandas as pd
import seaborn as sb
from enum import Enum
import matplotlib.pyplot as plt


class DataType(Enum):
    DEFAULT = 0
    LANGUAGE = 1
    IMAGE = 2
    SOUND = 3
    VIDEO = 4
    OTHER = 5
    
class DGE:
    def __init__(self, generator):
        self.generator = generator

    '''
    shuffles the data read from `in_file_path` and writes back to `out_file_path`.
    if `out_file_path` is None the original file is overwritten.
    `chunk_size` specifies the chunks to be shuffes within the file.
    if `chunk_size` is None the data is shuffled randomly.
    '''
    def shuffle(self, in_file_path, out_file_path=None, chunk_size=None):
        pass

    '''
    generates data by using `self.generator`.
    the generated data is written into `out_file_path` whose size does not exceed
    the `max_size_mb' MB.
    '''
    def generate(self, out_file_path, max_size_mb):
        if self.generator is None:
            print('Invalid generator!')
            return False
        pass


engine = DGE(None)
engine.generate(out_file_path='test.out', max_size_mb=0)

print(DataType.VIDEO, DataType.OTHER)

