import os
import sys


def sync_dirs(src_dir: str, dest_dir: str):
    src_child_files = os.listdir(src_dir)

    for src_child_file in src_child_files:
        src_child_fpath = os.path.join(src_dir, src_child_file)
        dest_child_fpath = os.path.join(dest_dir, src_child_file)

        if os.path.isdir(src_child_fpath):
            if not os.path.exists(dest_child_fpath):
                os.makedirs(dest_child_fpath)

            sync_dirs(
                src_dir=src_child_fpath,
                dest_dir=dest_child_fpath,
            )

        elif os.path.isfile(src_child_fpath):
            if not os.path.exists(dest_child_fpath):
                infile = open(src_child_fpath, mode='rb')
                outfile = open(dest_child_fpath, mode='wb')
                outfile.write(infile.read())
            else:
                # TODO should we check for the file content and overwrite it
                print('Skipping ' + dest_child_fpath)


def main():
    assets_directory = os.path.join(
        os.path.dirname(__file__),
        '..',
        'dependencies',
    )

    assets_directory = os.path.abspath(assets_directory)

    if not os.path.exists(assets_directory):
        print('Assets directory (' + assets_directory + ') does not exist!', file=sys.stderr)
        return -1

    savedmodel_dirname = 'etl9b.model.tensorflow.savedmodel'
    savedmodel_directory = os.path.join(assets_directory, savedmodel_dirname)

    if not os.path.exists(savedmodel_directory):
        print('Missing SavedModel at ' + savedmodel_directory, file=sys.stderr)
        return -1

    tf_dll_name = 'tensorflow.dll'
    tf_dll_fpath = os.path.join(assets_directory, 'lib', tf_dll_name)

    if not os.path.exists(tf_dll_fpath):
        print('Missing ' + tf_dll_name + ' at ' + tf_dll_fpath, file=sys.stderr)
        return -1

    output_directory = os.path.join(
        os.path.dirname(__file__),
        '..',
        'x64',
        'Debug',
    )

    output_directory = os.path.abspath(output_directory)

    print(output_directory)

    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    savedmodel_dest_path = os.path.join(output_directory, savedmodel_dirname)

    if not os.path.exists(savedmodel_dest_path):
        os.makedirs(savedmodel_dest_path)

    sync_dirs(savedmodel_directory, savedmodel_dest_path)

    tf_dll_dest_path = os.path.join(output_directory, tf_dll_name)

    if os.path.exists(tf_dll_dest_path):
        print('Skipping ' + tf_dll_dest_path)
    else:
        infile = open(tf_dll_fpath, mode='rb')
        outfile = open(tf_dll_dest_path, mode='wb')

        outfile.write(infile.read())

        infile.close()
        outfile.close()

    return 0


if __name__ == '__main__':
    main()
