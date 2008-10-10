#!/bin/sh

[ "$USERNAME" == "" ] && [ $(whoami) == az ] && USERNAME=albertzeyer

sftp $USERNAME@frs.sourceforge.net
