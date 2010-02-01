killall -INT openlierox dedicated_control recordmydesktop Xvfb jackd dedicated-video-record.sh
sleep 3
killall -KILL openlierox dedicated_control recordmydesktop Xvfb jackd dedicated-video-record.sh
sleep 2

cd share/gamedir
setsid nohup ../../bin/openlierox -dedicated > ../../dedicated.log &
sleep 10
ps -o pid --no-heading -C openlierox -C dedicated_control > ded_main_pids.pid
