import os
import zipfile
from urllib.request import urlretrieve

dependencies_dl_dir = 'dependencies'

if not os.path.exists(dependencies_dl_dir):
    os.makedirs(dependencies_dl_dir)

# Download tensorflow library

tf_lib_archive_url = 'https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-2.3.1.zip'

tf_lib_archive_filename = tf_lib_archive_url[tf_lib_archive_url.rfind('/')+1:]
tf_lib_archive_dl_fpath = os.path.join(dependencies_dl_dir, tf_lib_archive_filename)

if not os.path.exists(tf_lib_archive_dl_fpath):
    print('Downloading', tf_lib_archive_filename)
    print(tf_lib_archive_url)
    urlretrieve(tf_lib_archive_url, tf_lib_archive_dl_fpath)

zip_ref = zipfile.ZipFile(tf_lib_archive_dl_fpath, 'r')

required_files = []

for zip_file_ref in zip_ref.filelist:
    if zip_file_ref.filename.startswith('lib') or zip_file_ref.filename.startswith('include'):
        required_files.append(zip_file_ref)

for zip_file_ref in required_files:
    extract_dest = os.path.join(dependencies_dl_dir, zip_file_ref.filename)
    if not os.path.exists(extract_dest):
        print('Inflating', extract_dest)

        extract_dependencies_dl_dir, _ = os.path.split(extract_dest)

        if not os.path.exists(extract_dependencies_dl_dir):
            os.makedirs(extract_dependencies_dl_dir)

        with open(extract_dest, mode='wb') as outfile:
            outfile.write(zip_ref.read(zip_file_ref.filename))

    else:
        print('Skipping', extract_dest)

zip_ref.close()

# Download my own model for using load SavedModel API

savedmodel_archive_url = 'https://github.com/ichisadashioko/kanji-recognition/releases/download/v0.2/etl9b.model.tensorflow.savedmodel.zip'
savedmodel_archive_filename = savedmodel_archive_url[savedmodel_archive_url.rfind('/')+1:]
savedmodel_archive_dl_fpath = os.path.join(dependencies_dl_dir, savedmodel_archive_filename)

if not os.path.exists(savedmodel_archive_dl_fpath):
    print('Downloading', savedmodel_archive_filename)
    print(savedmodel_archive_url)
    urlretrieve(savedmodel_archive_url, savedmodel_archive_dl_fpath)

savedmodel_dir_name = 'etl9b.model.tensorflow.savedmodel'
savedmodel_dir_extracting_fpath = os.path.join(dependencies_dl_dir, savedmodel_dir_name)

if not os.path.exists(savedmodel_dir_extracting_fpath):
    print('Inflating ' + savedmodel_dir_extracting_fpath)

    zip_ref = zipfile.ZipFile(savedmodel_archive_dl_fpath, 'r')
    zip_ref.extractall(path=dependencies_dl_dir)
    zip_ref.close()
