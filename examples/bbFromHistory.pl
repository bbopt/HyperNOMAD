#!/usr/bin/perl
## use strict;

use warnings;

use Data::Dumper;

open (X, '<', "$ARGV[1]") or die("Impossible de lire $ARGV[1], $!\n");
@vectX=split(/\s+/,<X>);

my $dimPb = scalar @vectX;
my $nbOutputs = 1;
my $nameCacheFile="history_log.txt";
my $epsilon= 1.e-14;  # precision for matching points
my $dimVectX= scalar @vectX;
my $bbCommand = "./../src/blackbox/pytorch_bb.py Fashion-MNIST $ARGV[0]";

open (HISTORY, '<', "$nameCacheFile") or die("Impossible de lire $nameCacheFile, $!\n");
@PointsInCache = <HISTORY>;
foreach $Point (@PointsInCache){
        my @vectH=split(/\s+/,$Point);
        $match=0;
        # print "$Point \n";
        if ( scalar @vectH - $nbOutputs == $dimPb ) {
            for ($i = 0 ; $i < $dimPb ; $i++) {
                if ( abs($vectH[$i]-$vectX[$i]) > $epsilon ) {
                    $match=1;
                    last;
                }
            }
        }
        else {
            $match=1;
        }
        if ( $match==0 ) {
                for ($i=0 ; $i < $nbOutputs ; $i++)
                {
                        print "$vectH[$i+$dimPb] \n";
                }
                last;
        }
}
close (HISTORY);


if ( $match==1 ) {
#print "Match not found for $ARGV[0] in cache $nameCacheFile \n";
##open(BBOUT,"$bbCommand |") or die "Can't run program: $!\n";
##print <BBOUT>;
##close(BBOUT)
print "10000000\n";
}
