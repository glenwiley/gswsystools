#gswsystools

This is a collection of small system tools and scripts that are too tiny
to justify a separate repo for each.  This includes:

* binclock: bash script that displays a running clock in binary
* errno.c: prompts for an errno number and prints the errno string
* gendigs: performs dig against a list of zones and stores the results and packet captures
* genzones: generate DNS zone files (BIND) to help ptu together autoritative servers for testing DNS related sofware - this is NOT for building production zones
* getsolnics: prints a list of NICs on Solaris by querying kstat
* getsyscfg: collects a pile of system config files from a remote host
* ipaddr: shell script used to convert ip addresses to integers and back
* memhog.c: used to stress the system by consuming memory
* mkbakdt: bash function used to create a dated backup of a file and manage other backups of that file
* mkdmg: create an OSX dmg (an aid to packaging)
* mkusers: creating a list of users in bulk
* mvuser: shell script that packages/unpackages user credentials for moving a user to a different host
* nddget: Solaris script for fetching device configuration parameters
* pinghosts: ping a list of hosts in a way that lends itself to monitoring
* shmrm: remove all of the shared memory associate with a user
* shmtst.c: exercise shared memory access for basic benchmarking
* sizes.c: simple program to print the size (in bytes) of some fundamental data types
* spincpu.c: make a machine busy
* sshhosts: run a command via ssh on a list of hosts
* stopwatch: bash script count up timer and countdown timer
* whitelist: bash script that extracts addresses from a mail message and adds them to a whitelist

I provide more details on each tool below, the platforms list indicates which platforms I have actually used the tools on, they may work on many others but just haven't been tried by me.

#binclock

```
gwiley-reisende:gswsystools gwiley$ ./binclock -?
USAGE: binclock [-d]

-d   display date and time (default is time, no date)

Display a running clock in binary.

Glen Wiley <glen.wiley@gmail.com>
```

```
gwiley-reisende:gswsystools gwiley$ ./binclock

00000111 00101101 00000001

gwiley-reisende:gswsystools gwiley$ ./binclock -d

00001101/00001010/00011100 00000111 00101101 00010110
```

##Platforms:
* Linux
* OSX 10.8
* Solaris

#errno.c

Display the string associated with a specific errno number.  You could just take a
look at the errno headers on your system, but sometimes that is a bit of a pain.

```
gwiley-reisende:gswsystools gwiley$ ./errno
errno? 13
errno 13: Permission denied
errno? 
```

##Platforms:
* Linux
* OSX 10.8
* Solaris

#getsolnics

Query the kstat interface to determine what network interfaces are available on
the platform.

##Platforms:
* Solaris

#ipaddr

```
gwiley-reisende:gswsystools gwiley$ ./ipaddr -?
USAGE: ipaddr -a <ip1> <ip2>...
-a   convert each ip to an integer (as inet_aton())
-n   convert each integer to an ip (as inet_ntoa())
```

```
gwiley-reisende:gswsystools gwiley$ ./ipaddr -a 192.168.1.1 10.0.0.1
192.168.1.1 3232235777
10.0.0.1 167772161

gwiley-reisende:gswsystools gwiley$ ./ipaddr -n 3232235777 167772161
192.168.1.1 3232235777
10.0.0.1 167772161
```

##Platforms:
* Linux
* OSX 10.8
* Solaris

#memhog.c

Allocate memory to stress the system, good for testing behavior of applications
when memory is exhausted.

```
gwiley-reisende:gswsystools gwiley$ ./memhog -?
USAGE: memhog [-wuv] -s <kbytes>

  -s <kbytes>  allocate <kbytes> on the heap
  -w           allocate then wait - do not exit (use CTRL-C to exit)
  -u           use the memory (via memcpy)
  -v           be verbose

program will exit with 0 if the memory was successfully allocated

```

##Platforms:
* Linux
* OSX 10.8
* Solaris

#mvuser

Bash script that packages and unpackages user credentials, can be used to move
a user account from one host to another.

```
gwiley-reisende:gswsystools gwiley$ ./mvuser
USAGE: mvuser [-nf] [-H <home>] -p|i -g <group> | <user1> <user2> ... <usern>
-f             use with -i to continue in spite of existing users
-H <home>      use an alternative root home dir during -i/install
-i             install user data on current host
-p             package user data from current host
-n             no backups of /etc files (be careful)
-g <group>     use with -p, package all users in the specified group
-f             force during -i, ignore pre-existing users

Each username specified on the command line is packaged or
installed (depending on whether -i or -p is specified.

To copy an account on host A to host B you might:
  root@hostA# mvuser -p workerbee
  root@hostA# scp workerbee.user hostB:.
  root@hostA# ssh hostB
  root@hostB# mvuser -i workerbee
```

##Platforms
* Linux
* Solaris

#shmtst.c

Exercise basis shared memory access for simple benchmarking.

```
gwiley-reisende:gswsystools gwiley$ ./shmtst -?
USAGE: shmtst [-v] [-i <niter>] [-k <file>] [-p <proj>] -s <size>

  -i <niter>  number of iterations, default=5
  -k <file>   file name used to generate shm key, default=this executable
  -p <proj>   project number used to generate shm key, default=0
  -s <size>   shared memory size in bytes
              (specify K, M, G for kilo, mega, giga bytes)

  -v          verbose, display more diagnostic messages

```

```
gwiley-reisende:gswsystools gwiley$ ./shmtst -s 10K
Service Test             Num Ops   Time(usec)      Ops/Sec
heap    wrt_sm_seq     207500415      5009677      4141992
heap    rd_sm_seq      207500415      5010934      4140953
sysvshm wrt_sm_seq     204500409      5011350      4080745
sysvshm rd_sm_seq      201000402      5003166      4017464
heap    wrt_sm_seq     206000412      5008681      4112867
heap    rd_sm_seq      204000408      5007445      4073942
sysvshm wrt_sm_seq     204000408      5002645      4077851
sysvshm rd_sm_seq      207000414      5008670      4132842
heap    wrt_sm_seq     206000412      5007495      4113842
heap    rd_sm_seq      207000414      5001292      4138939
sysvshm wrt_sm_seq     207500415      5006861      4144321
sysvshm rd_sm_seq      207000414      5004566      4136231
heap    wrt_sm_seq     207500415      5005852      4145157
heap    rd_sm_seq      208000416      5010308      4151450
sysvshm wrt_sm_seq     207500415      5005490      4145457
sysvshm rd_sm_seq      207500415      5001347      4148891
heap    wrt_sm_seq     207000414      5005642      4135342
heap    rd_sm_seq      207000414      5003041      4137492
sysvshm wrt_sm_seq     207500415      5008448      4143008
sysvshm rd_sm_seq      207500415      5002841      4147652
```

##Platforms
* Linux
* OSX 10.8
* Solaris

#sizes.c

Print the sizes of some fundamental types.

```
gwiley-reisende:gswsystools gwiley$ uname -a
Darwin gwiley-reisende.local 10.8.0 Darwin Kernel Version 10.8.0: Tue Jun  7 16:32:41 PDT 2011; root:xnu-1504.15.3~1/RELEASE_X86_64 x86_64

gwiley-reisende:gswsystools gwiley$ ./sizes
char          : 1
short int     : 2
int           : 4
float         : 4
long          : 8
long long     : 8
double        : 8
```

##Platforms
* Linux
* OSX 10.8
* Solaris

#stopwatch
bash script that displays a running timer counting up or a countdown timer that
can beep when it hits 0

```
gwiley-reisende:gswsystools gwiley$ ./stopwatch -?
USAGE: stopwatch [-t] [-b <beeps>] [-d <seconds>]

-b <beeps>   play the specified number of bells/beeps when done
-d <seconds> count down specified seconds
-t           show elapsed time in hr/min/sec rather than just seconds

Simple stop watch and countdown timer. Default mode is to count
elapsed time in seconds (press CTL-C to exit).

Glen Wiley <glen.wiley@gmail.com>
```

##Platforms
* Linux
* OSX 10.8

#whitelist

bash script that extracts email addresses from a mail message and adds them to a whitelist file

##Platforms
* Linux
