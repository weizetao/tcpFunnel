xTproxy
======
A tcp proxy server, support upstream connection queue & load balance!

Require
------
* libev
* iksemel

Install
------
```
cd xtproxy
cmake .
make
```

Config
------
```
<conf>
    <pidfile>xtp.pid</pidfile>
    <server>
        <access port="9001" timeout="60" max="600000"></access>
        <upstream max="1000">
             <server ip="127.0.0.1" port="8081" timeout="10"></server>
             <server ip="127.0.0.1" port="8080" timeout="10"></server>
        </upstream>
    </server>
</conf>
```

Run
------
```
./bin/xtp -h
    -c           the path of config file
    -d           run as a deamon
    -L <path>    log path
    -l <level>   debug log_levels:DEBUG,WARN,INFO,ERR,NONE
    -h           print this help and exit
```
