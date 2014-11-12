#!/bin/sh

PATH=$PATH:/sbin:/usr/sbin:/usr/local/sbin

umask 077
rm -rf us
mkdir us
cd us

ssh-keygen -f host_rsa -t rsa -b 2048 -N ''
ssh-keygen -f client_rsa -t rsa -b 2048 -N ''
cp client_rsa.pub authorized_keys

cat > usd_config << EOF
AddressFamily inet
AllowTcpForwarding yes
AllowUsers $(whoami)
AuthorizedKeysFile $(pwd)/authorized_keys
ChallengeResponseAuthentication no
Ciphers aes128-ctr,aes192-ctr,aes256-ctr,aes192-cbc,aes256-cbc
DenyGroups
DenyUsers
ForceCommand /bin/sleep 3h
GSSAPIAuthentication no
HostbasedAuthentication no
HostKey $(pwd)/host_rsa
KerberosAuthentication no
LogLevel QUIET
MaxSessions 100
MaxStartups 10
PasswordAuthentication no
PidFile usd.pid
Port 1443
PrintMotd no
Protocol 2
PubkeyAuthentication yes
PubkeyAgent
UseDNS no
UseLogin no
UsePAM no
UsePrivilegeSeparation no
X11Forwarding no
EOF

ln -sf $(which sshd) usd
cat > run_usd.sh << EOF
#!/bin/sh

$(pwd)/usd -f usd_config "\$@"
EOF

chmod 700 run_usd.sh
