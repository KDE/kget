#!/usr/bin/perl

$doChange=0;
$connections=0;

while(<>)
{
    ($key)   = ($_ =~ /([^=]*)=(.*)$/);
    ($value) = ($_ =~ /^[^=]*=(.*)$/);

    if( $key eq "LimitDownloads" )
    {
        if ( $value eq "false" )
        {
            $doChange=1;
        }
        print "# DELETE " . $key . "\n";
        next;
    }
    if ( $key eq "MaxConnections" )
    {
        $connections=$value;
        print "# DELETE MaxConnections\n";
        next;
    }

    print $_;
}

if ( $doChange eq 1 )
{
    print "MaxConnections=0\n";
} else {
    print "MaxConnections=" . $connections . "\n";
}

