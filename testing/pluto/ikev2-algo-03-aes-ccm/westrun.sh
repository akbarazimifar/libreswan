ipsec auto --up westnet-eastnet-ipv4-psk-ikev2-ccm-a
../../guestbin/ping-once.sh --up -I 192.0.1.254 192.0.2.254
../../guestbin/ipsec-look.sh
echo done
