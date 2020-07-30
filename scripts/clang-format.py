#!/usr/bin/env python3
# encoding=utf-8

import os
import subprocess
import posixpath
import threading
from typing import List


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


C_SOURCE_EXTENSIONS = [
    '.h',
    '.c',
    '.cc',
]


def is_c_source_file(path: str):
    _, ext = posixpath.splitext(path)
    ext = ext.lower()
    if ext in C_SOURCE_EXTENSIONS:
        return True
    else:
        return False


def convert_string(bytes_input: bytes):
    encoding, s = Encoding.decode(bytes_input)

    if encoding == None:
        return str(bytes_input)
    else:
        return s


class Command:
    def __init__(self, cmd: List[str]):
        self.cmd = cmd

        # type is hinted implicitly (subprocess.Popen)
        self.p = None

        # The process is terminated by us because it took too long.
        # If this flag is True then the output is broken.
        self.terminated = False
        self.stdout = None
        self.stderr = None

    def target(self):
        print('>', ' '.join(self.cmd))
        self.process = subprocess.Popen(
            self.cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        self.stdout, self.stderr = self.process.communicate()

    def run(self, timeout=5, raise_on_error=True):
        thread = threading.Thread(target=self.target)
        thread.start()
        thread.join(timeout)

        if thread.is_alive():
            self.terminated = True
            self.process.terminate()
            # TODO Will call block our main thread for a long time?
            thread.join()

        if raise_on_error:
            if self.terminated:
                raise Exception(
                    f'The process is terminated because it took too long to excute!\n'
                    f'{convert_string(self.process.stderr)}'
                )

            if self.process.returncode != 0:
                raise Exception(f'')

            # if self.stdout is not None:
            #     output = convert_string(self.stdout)
            #     print(output)


if __name__ == '__main__':
    completed_process = subprocess.run(
        ['git', 'ls-files'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    lines = completed_process.stdout.decode('utf-8').split('\n')

    filepaths = filter(lambda x: len(x) > 0, lines)
    filepaths = filter(is_c_source_file, filepaths)
    filepaths = list(filepaths)

    print(filepaths)
    for filepath in filepaths:
        file_content = Encoding.decode(open(filepath, mode='rb').read())

        cmd = ['clang-format', '-style=file', filepath]
        sp = Command(cmd)
        sp.run()
        if sp.stdout is not None:
            clang_formatted_content = convert_string(sp.stdout)
            # TODO add 'check' or 'format' flags

            if file_content != clang_formatted_content:
                utf8_encoded_content = clang_formatted_content.encode('utf-8')
                open(filepath, mode='wb').write(utf8_encoded_content)
