#!/usr/bin/perl

use POSIX;
use File::Find;

my $rootdir = '';
my $output = '';
my %hash = ();
my $clines = 0;

#Language Files
if (!$ARGV[0] or $ARGV[0] eq "miranda") {
    create_langfile('../../Miranda-IM', '../../Miranda-IM/docs/translations/langpack_english.txt', 'English (UK)', '0809', 'Miranda IM Development Team', 'info@miranda-im.org');
}
if (!$ARGV[0] or $ARGV[0] eq "aim") {
create_langfile('../../Protocols/AIMToc/', '../../Protocols/AIMToc/docs/aim-translation.txt', 'English (UK)', '0809', 'Robert Rainwater', 'info@miranda-im.org');
}
if (!$ARGV[0] or $ARGV[0] eq "icq") {
    create_langfile('../../Protocols/IcqOscar8/', '../../Protocols/IcqOscar8/docs/IcqOscar8-translation.txt', 'English (UK)', '0809', 'Martin �berg, Richard Hughes, Jon Keating', 'info@miranda-im.org');
}
if (!$ARGV[0] or $ARGV[0] eq "changeinfo") {
    create_langfile('../../Plugins/changeinfo/', '../../Plugins/changeinfo/Docs/changeinfo-translation.txt', 'English (UK)', '0809', 'Richard Hughes', 'info@miranda-im.org');
}
if (!$ARGV[0] or $ARGV[0] eq "srmm") {
    create_langfile('../../Plugins/SRMM/', '../../Plugins/SRMM/Docs/srmm-translation.txt', 'English (UK)', '0809', 'Miranda IM Development Team', 'info@miranda-im.org');
}
if (!$ARGV[0] or $ARGV[0] eq "import") {
    create_langfile('../../Plugins/Import/', '../../Plugins/Import/docs/import-translation.txt', 'English (UK)', '0809', 'Martin �berg', 'info@miranda-im.org');
}

sub create_langfile {
    $rootdir = shift(@_);
    $outfile = shift(@_);
    $lang = shift(@_);
    $locale = shift(@_);
    $author = shift(@_);
    $email = shift(@_);
    %hash = ();
    $clines = 0;
    $output = "";
    print "Building language file for $rootdir:\n";
    find({ wanted => \&csearch, preprocess => \&pre_dir }, $rootdir);
    find({ wanted => \&rcsearch, preprocess => \&pre_dir }, $rootdir);
    open(WRITE, "> $outfile") or die;
    print WRITE "Miranda Language Pack Version 1\n";
    print WRITE "Locale: $locale\n";
    print WRITE "Authors: $author\n";
    print WRITE "Author-email: $email\n";
    print WRITE "Last-Modified-Using: Miranda IM 0.3.3\n";
    print WRITE "Plugins-included:\n\n";
    print WRITE "; Generated by lpgen on ".localtime()."\n";
    print WRITE "; Translations: $clines\n\n";
    print WRITE $output;
    close(WRITE);
    print "  $outfile is complete ($clines)\n\n";
}

sub pre_dir {
  @files = grep { not /^\.\.?$/ } @_;
  return sort @files;
}

sub append_str {
    $str = shift(@_);
    $found = shift(@_);
    $str = substr($str, 1, length($str) - 2);
    if (length($str) gt 0 and $str ne "List1" and $str ne "Tree1") {
        if (!$hash{$str}) {
            if ($found eq 0) {
                $output .= "; ".$file."\n";
            }
            $output .= ";[".$str."]\n";
            $hash{$str} = 1;
            $clines += 1;
            return 1;
        }
    }
    return 0;
}

sub csearch {
    if ( -f $_ and ($_ =~ m/\.c[pp]*\Z/ or $_ =~ m/\.h*\Z/)) {
        $found = 0;
        $file = $_;
        print "  Processing $_ ";
	    open(READ, "< $_") or return;
        $all = "";
	    while ($lines = <READ>) {
            $all = $all.$lines;
        }
	    close(READ);
        $_ = $all;
        while (/Translate\s*\(\s*(\".*?\")\s*\)/g) {
            $found += append_str($1, $found);
        }
        if ($found gt 0) {
            $output .= "\n";
        }
        print "($found)\n";
    }
}

sub rcsearch {
    if ( -f $_ and $_ =~ m/\.rc\Z/) {
        $found = 0;
        $file = $_;
        print "  Processing $_ ";
	    open(READ, "< $_") or return;
        $all = "";
	    while ($lines = <READ>) {
            $all = $all.$lines;
        }
	    close(READ);
        $_ = $all;
        while (/\s*CONTROL\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*DEFPUSHBUTTON\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*PUSHBUTTON\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*LTEXT\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*RTEXT\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*GROUPBOX\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*CAPTION\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        while (/\s*MENUITEM\s*(\".*?\")/g) {
            $found += append_str($1, $found);
        }
        if ($found gt 0) {
            $output .= "\n";
        }
        print "($found)\n";
    }
}