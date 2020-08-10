#!/bin/bash

cat <<- EOH
# Automated build of stochas

For more information see https://surge-synth-team.org/stochas/

EOH
date
echo ""
echo "Most recent commits:" 
echo ""
git log --pretty=oneline | head -5
