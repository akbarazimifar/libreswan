if [ -f /var/run/charon.pid -o -f /var/run/strongswan/charon.pid ]; then strongswan status ; fi
: ==== cut ====
if [ -f /var/run/pluto/pluto.pid ]; then ipsec look ; fi # ../guestbin/ipsec-look.sh
if [ -f /var/run/pluto/pluto.pid ]; then ipsec auto --status ; fi
if [ -f /var/run/charon.pid -o -f /var/run/strongswan/charon.pid ]; then strongswan statusall ; fi
: ==== tuc ====
if [ -f /sbin/ausearch ]; then ausearch -r -m avc -ts recent ; fi
