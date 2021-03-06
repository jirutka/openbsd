#	$OpenBSD: src/regress/usr.bin/ssh/cert-hostkey.sh,v 1.9 2014/01/26 10:22:10 djm Exp $
#	Placed in the Public Domain.

tid="certified host keys"

rm -f $OBJ/known_hosts-cert $OBJ/host_ca_key* $OBJ/cert_host_key*
cp $OBJ/sshd_proxy $OBJ/sshd_proxy_bak

HOSTS='localhost-with-alias,127.0.0.1,::1'

# Create a CA key and add it to known hosts
${SSHKEYGEN} -q -N '' -t rsa  -f $OBJ/host_ca_key ||\
	fail "ssh-keygen of host_ca_key failed"
(
	printf '@cert-authority '
	printf "$HOSTS "
	cat $OBJ/host_ca_key.pub
) > $OBJ/known_hosts-cert

PLAIN_TYPES=`$SSH -Q key-plain | sed 's/^ssh-dss/ssh-dsa/g;s/^ssh-//'`

type_has_legacy() {
	case $1 in
		ed25519*|ecdsa*) return 1 ;;
	esac
	return 0
}

# Generate and sign host keys
for ktype in $PLAIN_TYPES ; do 
	verbose "$tid: sign host ${ktype} cert"
	# Generate and sign a host key
	${SSHKEYGEN} -q -N '' -t ${ktype} \
	    -f $OBJ/cert_host_key_${ktype} || \
		fail "ssh-keygen of cert_host_key_${ktype} failed"
	${SSHKEYGEN} -h -q -s $OBJ/host_ca_key \
	    -I "regress host key for $USER" \
	    -n $HOSTS $OBJ/cert_host_key_${ktype} ||
		fail "couldn't sign cert_host_key_${ktype}"
	type_has_legacy $ktype || continue
	cp $OBJ/cert_host_key_${ktype} $OBJ/cert_host_key_${ktype}_v00
	cp $OBJ/cert_host_key_${ktype}.pub $OBJ/cert_host_key_${ktype}_v00.pub
	verbose "$tid: sign host ${ktype}_v00 cert"
	${SSHKEYGEN} -t v00 -h -q -s $OBJ/host_ca_key \
	    -I "regress host key for $USER" \
	    -n $HOSTS $OBJ/cert_host_key_${ktype}_v00 ||
		fail "couldn't sign cert_host_key_${ktype}_v00"
done

# Basic connect tests
for privsep in yes no ; do
	for ktype in $PLAIN_TYPES rsa_v00 dsa_v00; do 
		verbose "$tid: host ${ktype} cert connect privsep $privsep"
		(
			cat $OBJ/sshd_proxy_bak
			echo HostKey $OBJ/cert_host_key_${ktype}
			echo HostCertificate $OBJ/cert_host_key_${ktype}-cert.pub
			echo UsePrivilegeSeparation $privsep
		) > $OBJ/sshd_proxy

		${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
		    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
			-F $OBJ/ssh_proxy somehost true
		if [ $? -ne 0 ]; then
			fail "ssh cert connect failed"
		fi
	done
done

# Revoked certificates with key present
(
	printf '@cert-authority '
	printf "$HOSTS "
	cat $OBJ/host_ca_key.pub
	for ktype in $PLAIN_TYPES rsa_v00 dsa_v00; do
		test -f "$OBJ/cert_host_key_${ktype}.pub" || fatal "no pubkey"
		printf "@revoked * `cat $OBJ/cert_host_key_${ktype}.pub`\n"
	done
) > $OBJ/known_hosts-cert
for privsep in yes no ; do
	for ktype in $PLAIN_TYPES rsa_v00 dsa_v00; do 
		verbose "$tid: host ${ktype} revoked cert privsep $privsep"
		(
			cat $OBJ/sshd_proxy_bak
			echo HostKey $OBJ/cert_host_key_${ktype}
			echo HostCertificate $OBJ/cert_host_key_${ktype}-cert.pub
			echo UsePrivilegeSeparation $privsep
		) > $OBJ/sshd_proxy

		${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
		    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
			-F $OBJ/ssh_proxy somehost true >/dev/null 2>&1
		if [ $? -eq 0 ]; then
			fail "ssh cert connect succeeded unexpectedly"
		fi
	done
done

# Revoked CA
(
	printf '@cert-authority '
	printf "$HOSTS "
	cat $OBJ/host_ca_key.pub
	printf '@revoked '
	printf "* "
	cat $OBJ/host_ca_key.pub
) > $OBJ/known_hosts-cert
for ktype in $PLAIN_TYPES rsa_v00 dsa_v00 ; do 
	verbose "$tid: host ${ktype} revoked cert"
	(
		cat $OBJ/sshd_proxy_bak
		echo HostKey $OBJ/cert_host_key_${ktype}
		echo HostCertificate $OBJ/cert_host_key_${ktype}-cert.pub
	) > $OBJ/sshd_proxy
	${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
	    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
		-F $OBJ/ssh_proxy somehost true >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		fail "ssh cert connect succeeded unexpectedly"
	fi
done

# Create a CA key and add it to known hosts
(
	printf '@cert-authority '
	printf "$HOSTS "
	cat $OBJ/host_ca_key.pub
) > $OBJ/known_hosts-cert

test_one() {
	ident=$1
	result=$2
	sign_opts=$3

	for kt in rsa rsa_v00 ; do
		case $kt in
		*_v00) args="-t v00" ;;
		*) args="" ;;
		esac

		verbose "$tid: host cert connect $ident $kt expect $result"
		${SSHKEYGEN} -q -s $OBJ/host_ca_key \
		    -I "regress host key for $USER" \
		    $sign_opts $args \
		    $OBJ/cert_host_key_${kt} ||
			fail "couldn't sign cert_host_key_${kt}"
		(
			cat $OBJ/sshd_proxy_bak
			echo HostKey $OBJ/cert_host_key_${kt}
			echo HostCertificate $OBJ/cert_host_key_${kt}-cert.pub
		) > $OBJ/sshd_proxy
	
		${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
		    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
		    -F $OBJ/ssh_proxy somehost true >/dev/null 2>&1
		rc=$?
		if [ "x$result" = "xsuccess" ] ; then
			if [ $rc -ne 0 ]; then
				fail "ssh cert connect $ident failed unexpectedly"
			fi
		else
			if [ $rc -eq 0 ]; then
				fail "ssh cert connect $ident succeeded unexpectedly"
			fi
		fi
	done
}

test_one "user-certificate"	failure "-n $HOSTS"
test_one "empty principals"	success "-h"
test_one "wrong principals"	failure "-h -n foo"
test_one "cert not yet valid"	failure "-h -V20200101:20300101"
test_one "cert expired"		failure "-h -V19800101:19900101"
test_one "cert valid interval"	success "-h -V-1w:+2w"
test_one "cert has constraints"	failure "-h -Oforce-command=false"

# Check downgrade of cert to raw key when no CA found
for v in v01 v00 ;  do 
	for ktype in $PLAIN_TYPES ; do 
		type_has_legacy $ktype || continue
		rm -f $OBJ/known_hosts-cert $OBJ/cert_host_key*
		verbose "$tid: host ${ktype} ${v} cert downgrade to raw key"
		# Generate and sign a host key
		${SSHKEYGEN} -q -N '' -t ${ktype} \
		    -f $OBJ/cert_host_key_${ktype} || \
			fail "ssh-keygen of cert_host_key_${ktype} failed"
		${SSHKEYGEN} -t ${v} -h -q -s $OBJ/host_ca_key \
		    -I "regress host key for $USER" \
		    -n $HOSTS $OBJ/cert_host_key_${ktype} ||
			fail "couldn't sign cert_host_key_${ktype}"
		(
			printf "$HOSTS "
			cat $OBJ/cert_host_key_${ktype}.pub
		) > $OBJ/known_hosts-cert
		(
			cat $OBJ/sshd_proxy_bak
			echo HostKey $OBJ/cert_host_key_${ktype}
			echo HostCertificate $OBJ/cert_host_key_${ktype}-cert.pub
		) > $OBJ/sshd_proxy
		
		${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
		    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
			-F $OBJ/ssh_proxy somehost true
		if [ $? -ne 0 ]; then
			fail "ssh cert connect failed"
		fi
	done
done

# Wrong certificate
(
	printf '@cert-authority '
	printf "$HOSTS "
	cat $OBJ/host_ca_key.pub
) > $OBJ/known_hosts-cert
for v in v01 v00 ;  do 
	for kt in $PLAIN_TYPES ; do 
		type_has_legacy $kt || continue
		rm -f $OBJ/cert_host_key*
		# Self-sign key
		${SSHKEYGEN} -q -N '' -t ${kt} \
		    -f $OBJ/cert_host_key_${kt} || \
			fail "ssh-keygen of cert_host_key_${kt} failed"
		${SSHKEYGEN} -t ${v} -h -q -s $OBJ/cert_host_key_${kt} \
		    -I "regress host key for $USER" \
		    -n $HOSTS $OBJ/cert_host_key_${kt} ||
			fail "couldn't sign cert_host_key_${kt}"
		verbose "$tid: host ${kt} connect wrong cert"
		(
			cat $OBJ/sshd_proxy_bak
			echo HostKey $OBJ/cert_host_key_${kt}
			echo HostCertificate $OBJ/cert_host_key_${kt}-cert.pub
		) > $OBJ/sshd_proxy
	
		${SSH} -2 -oUserKnownHostsFile=$OBJ/known_hosts-cert \
		    -oGlobalKnownHostsFile=$OBJ/known_hosts-cert \
			-F $OBJ/ssh_proxy -q somehost true >/dev/null 2>&1
		if [ $? -eq 0 ]; then
			fail "ssh cert connect $ident succeeded unexpectedly"
		fi
	done
done

rm -f $OBJ/known_hosts-cert $OBJ/host_ca_key* $OBJ/cert_host_key*
