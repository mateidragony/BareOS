# BareOS 🐻

This is my implementation of the very bare-bones operating system that was the semester long project of the Operating Systems course (CSCI-P 436) course at Luddy. This project was a very VERY fun learning experience and a great opportunity to mess around. I'd say that 50% of the code in this repository is assignment code, and the other 50% is me just having a grand old time.

## About

This project was split into 10 milestones that allowed us to slowly build up our operating system from a simple bootstrap file to a very limited but functional system with a shell, threads, dynamic memory, a tty, and a filesystem.

The each milestone is listed below:

1. **Standard Output**
    - Implement printf. (We can finally use `printf("here")` instead of gdb to debug 😁!!!)
    - Mind you at this point the OS has literally nothing, so we are using a `uart_putc` function that literally busy waits until the uart is free to print a new character to the screen 💀.
2. **The Shell**
    - Implement a shell that prints out a standard prompt `bareOS$` (or in my case `Mash$`) and allows users to type in builtin functions like `echo`.
    - This is the first milestone where I was able to have unlimited freedom. I did so many things with this shell that its better if you play around and see for yourself 🙃.
3. **Thread Creation**
    - Implement bareOS threads. The shell now creates and joins threads whenever the user types in a command.
4. **Thread Scheduling**
    - Implement a queue to schedule threads. Not much to say about this milestone. It's just a smarter way of scheduling the threads.
5. **Timers and Sleep**
    - Implement a sleep delta queue to allow threads to sleep. Another staightforward milestone. Don't worry, it gets hype soon!
6. **Synchronization**
    - Semaphores!!! Due to time constraints with the class, this milestone was left as a bonus milestone to implement. This milestone really gave me a deep understanding of what semaphores were and how they work. I love synchronizing my concurrent threads.
7. **Dynamic Memory Allocation**
    - Malloc. Sidenote: using malloc without any MMU setup is very cursed...
8. **Interrupts and the UART**
    - Implement a TTY for the UART. Very hype. We no longer rely on the very terrible while loop busy wait `uart_getc` and we can now use the based semaphore-pilled resched-maxxing `tty_getc`.
9. **Filesystems Part 1**
    - Create user functions to create, open, and close files on the ramdisk filesystem. The hardest part of this milestone was reading the 50 pages of documentation Musser wrote for his implementation of the ramdisk 💀.
10. **Filesystems Part 2**
    - Reading and writing to files!!! Implementing vim is now in sight...