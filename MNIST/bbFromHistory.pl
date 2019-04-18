#!/usr/bin/perl
# use strict;
use warnings;

use Data::Dumper;

open (X, '<', "$ARGV[0]") or die("Impossible de lire $ARGV[0], $!\n");	
@vectX=split(/\s+/,<X>);


my $nbOutputs = 1;
my $nameCacheFile="history.0.txt";
my $epsilon= 1.e-14;  # precision for matching points
my $dimVectX= scalar @vectX;

my $bbCommand = "python ./pytorch_bb.py $ARGV[0]";

open (HISTORY, '<', "$nameCacheFile") or die("Impossible de lire $nameCacheFile, $!\n");	
@PointsInCache = <HISTORY>;
foreach $Point (@PointsInCache){
    
    $match=0;
    my @vectH=split(/\s+/,$Point);
    for ($i = 0 ; $i < $dimVectX ; $i++) {
        if ( abs($vectH[$i]-$vectX[$i]) > $epsilon ) {
            $match=1;
            last;
        }
    }
    if ( $match==0 ) {
        for ($i=0 ; $i < $nbOutputs ; $i++)
        {
            print "$vectH[$i+$dimVectX] \n";
        }
        last;
    }
    
}
close (HISTORY);

if ( $match==1 ) {
    #print "Match not found for $ARGV[0] in cache $nameCacheFile \n";
    open(BBOUT,"$bbCommand |") or die "Can't run program: $!\n";
    print <BBOUT>;
    close(BBOUT)
}



