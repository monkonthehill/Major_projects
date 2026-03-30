#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(IXON | ICRNL);

    /* NOTE: IXON is used to disable ctrl + s & q which are responsible for freezing the program and
    continuing it respectivly */

    raw.c_oflag &= (OPOST);

    /* NOTE: We will turn off all output processing features by turning off the OPOST flag. In
     practice, the "\n" to "\r\n" translation is likely the only output processing feature turned
     on by default.
    */

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    raw.c_cflag |= (CS8);

    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    /*NOTE: echo is for disabling the echo of the input ICANON is used for disabling the
      canonical mode of the terminal ISIG is to stop the signal of ctrl + z and c
    */

    /* NOTE: On some systems, when you type Ctrl-V, the terminal waits for you to type another
     character and then sends that character literally. For example, before we disabled Ctrl-C,
     you might’ve been able to type Ctrl-V and then Ctrl-C to input a 3 byte. We can turn off this
     feature using the IEXTEN flag.
    */

    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 1;

    /* NOTE:VMIN and VTIME come from <termios.h>. They are indexes into the c_cc field, which stands
     for “control characters”, an array of bytes that control various terminal settings.
     The VMIN value sets the minimum number of bytes of input needed before read()
        can return.We set it to 0 so that read() returns as soon as there is any input to be
            read.The VTIME value sets the maximum amount of time to wait before read()
                returns.It is in tenths of a                                    second,
        so we set it to 1 / 10 of a second, or 100 milliseconds.If read() times out,
        it                                                                      will return 0,
        which makes sense because its usual return value is the number of bytes read.
    */

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
    enableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        if (iscntrl(c))
        {
            printf("%d\n", c);
        }
        else
        {
            printf("%d ('%c')\n", c, c);
        }
    }
    return 0;
}
