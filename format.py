#!/usr/bin/env python3
# encoding=utf-8
# python3 -m pip install --user tqdm
import os
import mimetypes
import traceback
import subprocess
from subprocess import PIPE

from tqdm import tqdm


class Encoding:
    UTF8 = 'utf-8'
    UTF8_WITH_BOM = 'utf-8-sig'
    UTF16 = 'utf-16'
    GB2312 = 'gb2312'
    SHIFT_JIS = 'shift-jis'

    @classmethod
    def decode(cls, bs: bytes):
        try:
            encoding = cls.UTF8_WITH_BOM
            decoded_content = bs.decode(encoding)
            return encoding, decoded_content
        except Exception as ex:
            # traceback.print_exc()
            pass

        try:
            encoding = cls.UTF8
            decoded_content = bs.decode(encoding)
            return encoding, decoded_content
        except Exception as ex:
            # traceback.print_exc()
            pass

        try:
            encoding = cls.UTF16
            decoded_content = bs.decode(encoding)
            return encoding, decoded_content
        except Exception as ex:
            # traceback.print_exc()
            pass

        try:
            encoding = cls.GB2312
            decoded_content = bs.decode(encoding)
            return encoding, decoded_content
        except Exception as ex:
            # traceback.print_exc()
            pass

        try:
            encoding = cls.SHIFT_JIS
            decoded_content = bs.decode(encoding)
            return encoding, decoded_content
        except Exception as ex:
            # traceback.print_exc()
            pass

        return None, bs


skips = [
    '.git',  # git directory
    'logs',  # log directory
    'Backup',  # Visual Studio project migration files
    # known Visual Studio files
    'bin',
    'obj',
    '.vs',
]

skip_extensions = [
    '.bomb',
    '.map',
    # Microsoft Excel files
    '.xls',
    # known binary extensions
    '.dll',
    '.jpg',
    '.gif',
    '.png',
    # weird files from Visual Studio
    '.suo',
]


def find_all_files(infile):
    basename = os.path.basename(infile)
    if basename in skips:
        return []

    retval = []

    if os.path.isfile(infile):
        ext = os.path.splitext(infile)[1].lower()
        if ext in skip_extensions:
            return []
        else:
            return [infile]

    elif os.path.isdir(infile):
        flist = os.listdir(infile)
        for fname in flist:
            fpath = os.path.join(infile, fname)
            retval.extend(find_all_files(fpath))

    return retval


if __name__ == '__main__':
    # all files
    # file_list = find_all_files('.')

    # tracked files only
    completed_process = subprocess.run(
        ['git', 'ls-files'],
        stdout=PIPE,
        stderr=PIPE,
    )

    lines = completed_process.stdout.decode('utf-8').split('\n')

    file_list = list(filter(lambda x: len(x) > 0, lines))

    pbar = tqdm(file_list)
    for fpath in pbar:
        pbar.set_description(fpath)
        # mime = mimetypes.guess_type(fpath)
        # print(mime, fpath)

        if not os.path.exists(fpath):
            continue

        basename = os.path.basename(fpath)
        ext = os.path.splitext(basename)[1].lower()
        if ext in skip_extensions:
            continue

        bs = open(fpath, mode='rb').read()
        encoding, decoded_string = Encoding.decode(bs)

        if encoding is None:
            continue

        if not encoding == Encoding.UTF8:
            open(fpath, mode='w', encoding=Encoding.UTF8).write(decoded_string)

        # enforce LF line ending
        content = decoded_string.replace('\r\n', '\n')
        content = content.strip('\n')

        # append empty line at the end
        # it's good practice for Git
        content = content + '\n'

        os.remove(fpath)  # file will not be changed if we don't remove it
        with open(fpath, mode='wb') as outfile:
            encoded_content = content.encode(Encoding.UTF8)
            outfile.write(encoded_content)
        # print(encoding, fpath)
