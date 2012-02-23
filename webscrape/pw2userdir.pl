#!/usr/bin/perl -w
# $Id: pw2userdir.pl,v 1.2 2002/05/06 02:18:14 drench Exp $

use strict;
use Fcntl;

my $root = '/etc/users';
my @f = qw(NAME PASSWD UID GID CHANGE CLASS GECOS DIR SHELL EXPIRE FIELDS); 

if (-d $root) {
    die "$root already exists!\n";
}

mkdir($root, 0755) || die "Unable to mkdir $root: $!\n";
chown(0, 0, $root) || die "Unable to chown 0:0 $root\n";

die "$root must exist!" unless -d $root && chdir $root;

my @users;
while(my $u = getpwent()) { push @users, $u }

USER: foreach my $u (@users) {
    my %x;
    @x{@f} = getpwnam($u);

    my $name = $x{'NAME'};
    if ($name =~ /[^\w\-\!\"\#\$\%\&\'\(\)\*\+\,\.\:\;\?]/) {
        warn "'$name' contains odd characters; Skipping...\n";
        next USER;
    }
    if (-d $name) {
        warn "Login name '$name' already exists! Skipping...\n";
        next USER;
    } else {
        if (!mkdir $name, 0755) {
            warn "Unable to mkdir '$name': $!\nSkipping...\n";
            next USER;
        }
        if (!chown(0, 0, $name)) {
            warn "Unable to chown 0:0 '$name'; Skipping...\n";
            rmdir $name;
            next USER;
        }
    }

    my $uid = $x{'UID'};

    eval {
        FIELD: foreach my $field (@f) {
            next FIELD unless defined $x{$field};
            local *F;
            sysopen(F, $name.'/'.$field, (O_WRONLY | O_CREAT),
                    ($field eq 'PASSWD') ? 0400 : 0644)
                || die "Unable to open '$name/$field' for writing: $!";
            print F $x{$field};
            close(F) || die "trouble closing '$name/$field': $!";
            chown(($field eq 'GECOS') ? $uid : 0, 0, "$name/$field")
                || die "trouble chown-ing '$name/$field'\n";
        }
    };
    if ($@) {
        warn "$@\n";
    }
}
