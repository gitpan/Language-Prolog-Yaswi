use ExtUtils::MakeMaker;


print "retrieving SWI-Prolog configuration:\n";
my $pl=$ENV{PL} || 'pl';
print "  running '$pl -dump-runtime-variables'\n";
my @plvars=`$pl -dump-runtime-variables`;
die "unable to run swi-prolog command '$pl': $?\n" if $?;

my %plvar;
foreach (@plvars) {
    if (/^(PL.*?)="(.*)";$/) {
	$plvar{$1}=$2;
	print "    $1: $plvar{$1}\n";
    }
}

$plvar{PLLIBS}.=" -L$plvar{PLBASE}/runtime/$plvar{PLARCH}/ ";
$plvar{PLLIBS}.=($plvar{PLTHREADS} eq 'yes') ? '-lplmt' : '-lpl';

print "creating plconfig.c\n";
open (PLC, ">plconfig.c")
    or die "unable to open 'plconfig.c' to store SWI-Prolog configuration\n";
print PLC <<EOC;
char *PL_base="$plvar{PLBASE}";
char *PL_arch="$plvar{PLARCH}";
char *PL_exe="$plvar{PLBASE}/bin/$plvar{PLARCH}/pl";
EOC
close (PLC)
    or die "unable to write 'plconfig.c' to store SWI-Prolog configuration\n";

WriteMakefile(
    'NAME'		=> 'Language::Prolog::Yaswi::Low',
    'VERSION_FROM'	=> 'Low.pm',
    'PREREQ_PM'		=> { 'Language::Prolog::Yaswi' => '0.01' },
    'ABSTRACT_FROM'     => 'Low.pm',
    'AUTHOR'            => 'Salvador Fandiño <sfandino@yahoo.com>',
    'LIBS'		=> [$plvar{PLLIBS}],
    'DEFINE'		=> '',
    'INC'		=> "-I. -I$plvar{PLBASE}/include",
    'OBJECT'            => '$(BASEEXT)$(OBJ_EXT) callback.o callperl.o'.
	                   ' hook.o perl2swi.o swi2perl.o frames.o'.
	                   ' plconfig.o argv.o query.o vars.o',
#    'OPTIMIZE'		=> '-g',
);

