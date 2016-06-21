import os
import sys

target_dir = '.'

if len(sys.argv) > 1:
    target_dir = sys.argv[1]

for fl in os.listdir(target_dir):
    fp = os.path.join(target_dir,fl)
    if 'images_eye' in fl and '.log' in fl:
        with open(fp) as coord_file:
            ns = fl.split('_')
            rec_num = int(ns[1][3:])
            frame_num = int(ns[2][:-4])
            orig_log = 'log' + str(rec_num) + '_' + str(frame_num).zfill(4) + '.out'

            with open(orig_log,'r') as tmp:
                in_data = tmp.read().strip('\r\n').split(',')
            with open(orig_log[:-4] + '_new.out','w') as out_log:
                new_data = coord_file.read().replace('\n',',').split(',')
                out_log.write(','.join(in_data + new_data))
