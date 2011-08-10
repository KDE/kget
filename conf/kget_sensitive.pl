#!/usr/bin/perl


while(<>)
{
    ($key)   = ($_ =~ /([^=]*)=(.*)$/);
    ($value) = ($_ =~ /^[^=]*=(.*)$/);

    if( $key eq "AutoPasteCaseInsensitive" and $value eq "false" )
    {
        print "# DELETE " . $key . "\n";
        print "AutoPasteCaseSensitive=true\n";
        next;
    }

    print $_;
}
