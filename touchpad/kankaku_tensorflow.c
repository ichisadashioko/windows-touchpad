#include "kankaku_tensorflow.h"

#include <stdio.h>

#include <tensorflow/c/c_api.h>
#pragma comment(lib, "tensorflow.lib")

void kankaku_tensorflow_hello_world() {
  printf("Hello from TensorFlow C library version %s\n", TF_Version());
}
