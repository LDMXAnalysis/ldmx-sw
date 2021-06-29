# merge

This is separated from the rest of the module because it should be
able to be compiled separately on a system with only ROOT installed.

We focus here on creating an executable that can take several data streams
and merge them into one RawData ROOT file where the data is grouped by event
rather than by stream.

Started from hexactrl-sw which used Boost.Serialization for writing/reading
data from the HGC ROC.
