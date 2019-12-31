#!/bin/sh

rc=0

for f; do
    echo indenting $f ...
    if indent -kr $f -o $f.new; then
        mv $f.new $f
    else
        echo "$f: failed to indent"
        rc=1
    fi
done

exit $rc
