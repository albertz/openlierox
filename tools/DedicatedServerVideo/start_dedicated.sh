killall -INT openlierox dedicated_control recordmydesktop Xvfb jackd dedicated-video-record.sh
sleep 3
killall -KILL openlierox dedicated_control recordmydesktop Xvfb jackd dedicated-video-record.sh
sleep 2
cd share/gamedir
setsid nohup ../../bin/openlierox -dedicated > ../../dedicated.log &
sleep 5
pgrep -u $USER 'openlierox|dedicated_contr' > ded_main_pids.pid
