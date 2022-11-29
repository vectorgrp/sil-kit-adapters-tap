#!/bin/sh

# generate host keys
dpkg-reconfigure openssh-server

# allow login with password to root
sed -i -e 's/^[ \t#]*PermitRootLogin[ \t].*$/PermitRootLogin yes/' /etc/ssh/sshd_config

# enable and start the ssh service
systemctl enable ssh
systemctl restart ssh

