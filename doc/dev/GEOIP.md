# GeoIP database

`share/gamedir/GeoIP.dat` maps IPs to countries,
shown next to servers and players in the lobby.
It's read by [GeoIPDatabase.cpp](../../src/common/GeoIPDatabase.cpp),
a reader for MaxMind's old "GeoIP Legacy" binary format
(`GeoIP Country Edition`).

MaxMind stopped publishing that format in 2018
and retired the old download service in 2022;
they now only publish the newer GeoLite2 format (a different, incompatible layout).
The current file is a third-party rebuild of the legacy format
from MaxMind's current GeoLite2 data:

    curl -LO https://mailfud.org/geoip-legacy/GeoIP.dat.gz
    gunzip GeoIP.dat.gz
    mv GeoIP.dat share/gamedir/GeoIP.dat

That mirror (built with
[sherpya/geolite2legacy](https://github.com/sherpya/geolite2legacy))
is updated weekly; `https://www.miyuru.lk/geoiplegacy` is an alternative
mirror doing the same conversion, updated less often.
Since both are derived from GeoLite2 data,
the [GeoLite2 EULA](https://www.maxmind.com/en/geolite2/eula) applies.
