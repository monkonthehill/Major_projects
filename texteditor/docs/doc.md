
# FILE: vom.c
---

### Line 21 [CODENOTE] → echo is for disabling the echo of the input ICANON is used for disabling the
```cpp
    // CODENOTE: echo is for disabling the echo of the input ICANON is used for disabling the
    // canonical mode of the terminal ISIG is to stop the signal of ctrl + z and c

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
```
---
