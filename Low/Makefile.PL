use ExtUtils::MakeMaker;


print "retrieving SWI-Prolog configuration:\n";
my $defaultpl= $^O=~/Win32/ ? 'plcon.exe' : 'pl';
my $pl=$ENV{PL} || $defaultpl;
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

if ($^O=~/Win32/) {
    $plvar{PLLIBS}.=qq( -L"$plvar{PLBASE}/lib" -lpl);
    $plvar{PLINC}=qq(-I"$plvar{PLBASE}/include");
    $plvar{PLEXE}=qq($plvar{PLBASE}/bin/libpl.dll);
    $plvar{PLPATH}=qq($plvar{PLBASE}/bin);
}
else {
    $plvar{PLLIBS}.=" -L$plvar{PLBASE}/runtime/$plvar{PLARCH}/ ";
    $plvar{PLLIBS}.=($plvar{PLTHREADS} eq 'yes') ? '-lplmt' : '-lpl';
    $plvar{PLINC}="-I$plvar{PLBASE}/include";
    $plvar{PLEXE}="$plvar{PLBASE}/bin/$plvar{PLARCH}/pl";
    $plvar{PLPATH}="$plvar{PLBASE}/bin/$plvar{PLARCH}/";
}

print "creating plconfig.c\n";
open (PLC, ">plconfig.c")
    or die "unable to open 'plconfig.c' to store SWI-Prolog configuration\n";
print PLC <<EOC;
char *PL_base="$plvar{PLBASE}";
char *PL_arch="$plvar{PLARCH}";
char *PL_exe="$plvar{PLEXE}";
char *PL_path="$plvar{PLPATH}";
EOC
close (PLC)
    or die "unable to write 'plconfig.c' to store SWI-Prolog configuration\n";

WriteMakefile(
    'NAME'		=> 'Language::Prolog::Yaswi::Low',
    'VERSION_FROM'	=> 'Low.pm',
    'PREREQ_PM'		=> {},
    'ABSTRACT_FROM'     => 'Low.pm',
    'AUTHOR'            => 'Salvador Fandiño <sfandino@yahoo.com>',
    'LIBS'		=> [$plvar{PLLIBS}],
    'DEFINE'		=> '',
    'INC'		=> "-I. $plvar{PLINC}",
    'OBJECT'            => '$(BASEEXT)$(OBJ_EXT) callback$(OBJ_EXT) callperl$(OBJ_EXT)'.
	                   ' hook$(OBJ_EXT) perl2swi$(OBJ_EXT) swi2perl$(OBJ_EXT) frames$(OBJ_EXT)'.
	                   ' plconfig$(OBJ_EXT) argv$(OBJ_EXT) query$(OBJ_EXT) vars$(OBJ_EXT)',
#    'OPTIMIZE'		=> '-g',
);

