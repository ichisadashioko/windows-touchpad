import os
import zipfile
from urllib.request import urlretrieve

tf_lib_url = 'https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-2.3.1.zip'
dest_dir = 'dependencies'

if not os.path.exists(dest_dir):
    os.makedirs(dest_dir)

tf_lib_dest_filename = tf_lib_url[tf_lib_url.rfind('/')+1:]
tf_lib_dest_fpath = os.path.join(dest_dir, tf_lib_dest_filename)

if not os.path.exists(tf_lib_dest_fpath):
    print('Downloading', tf_lib_dest_filename)
    urlretrieve(tf_lib_url, tf_lib_dest_fpath)

zip_ref = zipfile.ZipFile(tf_lib_dest_fpath, 'r')

required_files = []

for zip_file_ref in zip_ref.filelist:
    if zip_file_ref.filename.startswith('lib') or zip_file_ref.filename.startswith('include'):
        required_files.append(zip_file_ref)

for zip_file_ref in required_files:
    extract_dest = os.path.join(dest_dir, zip_file_ref.filename)
    if not os.path.exists(extract_dest):
        print('Inflating', extract_dest)

        extract_dest_dir, _ = os.path.split(extract_dest)

        if not os.path.exists(extract_dest_dir):
            os.makedirs(extract_dest_dir)

        with open(extract_dest, mode='wb') as outfile:
            outfile.write(zip_ref.read(zip_file_ref.filename))

    else:
        print('Skipping', extract_dest)

zip_ref.close()
