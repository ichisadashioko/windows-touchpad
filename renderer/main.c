#include <Windows.h>

#include <stdio.h>

#define BUFSIZE 3  // bytes

int main()
{
  HANDLE hPipe        = INVALID_HANDLE_VALUE;
  LPCSTR lpszPipename = TEXT("\\\\.\\pipe\\touchpad");

  hPipe = CreateNamedPipe(lpszPipename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, BUFSIZE, BUFSIZE, 0, NULL);
  // TODO
  return 0;
}
