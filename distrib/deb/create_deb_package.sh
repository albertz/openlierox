#!/bin/bash

cd ../.. # go into root dir
dpkg-buildpackage -rfakeroot
