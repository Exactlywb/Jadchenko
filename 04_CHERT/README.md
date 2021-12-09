# Chert implementation (linux daemon)
Dedicated to the main daemon of 425th room - Timur.

## How to build

After git clone:
```
mkdir build
cd build/
cmake ..
make
./daemon src_file dest_file
```

## How to look log files

```
cd /var/log/
cat -n -e syslog
```

## How communicate Timur

Communication system with Timur has been built on signals. 

* SIGALRM

Don't touch it if you don't want to force a copy.
 
* SIGINT

If you want to change copy timeout. Send it using sigqueue.

* SIGUSR1

Change source dir.
Write your own path into ```newPath``` and then send SIGUSR1.

* SIGUSR2

Change destination dir. 
Write your own path into ```newPath``` and then send SIGUSR2.

* SIGQUIT

Kill Timur. A dream of many people.
