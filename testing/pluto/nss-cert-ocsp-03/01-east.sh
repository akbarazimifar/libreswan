/testing/guestbin/swan-prep --x509
certutil  -d sql:/etc/ipsec.d -D -n west
ipsec start
../../guestbin/wait-until-pluto-started
ipsec whack --impair suppress-retransmits
ipsec auto --add nss-cert-ocsp
ipsec auto --status |grep nss-cert-ocsp
echo "initdone"
