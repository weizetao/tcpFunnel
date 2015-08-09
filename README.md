xTproxy
======
A tcp proxy server, support upstream connection queue & load balance!

适用场景
------
* 短连接request-repsone服务(如HTTP services)
* 有的服务并发能力较弱，高并发出现时导致服务崩坏
* 设置后端最大并发数，超出的并不是refuse，而是cache住，排队处理
* 将瞬时高并发变成FIFO的序列请求，降低对后端服务器的瞬时压力
* 非常适合抢购、秒杀等瞬间高并发业务
* 不适合长连接的请求

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

