use ExtUtils::MakeMaker;our $AM_DEFAULT_VERBOSITY='0';require "../../../src/perl/Makefile_silent.pm";

WriteMakefile('NAME' => 'Irssi::TextUI',
              'LIBS' => '',
	      'OBJECT' => '$(O_FILES)',
              'TYPEMAPS' => ['../common/typemap', '../ui/typemap'],
              'INC' => '-I../../.. -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include',
	      'VERSION_FROM' => './TextUI.pm');
